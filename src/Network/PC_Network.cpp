#include "PC_Network.hpp"


// Namespace -----------------------------------------------------------------------------
using namespace std::chrono;
using namespace std::chrono_literals;


// Pre-Compiler Constants ----------------------------------------------------------------
#define IP_LEN 15


// Globals -------------------------------------------------------------------------------
std::chrono::milliseconds PACKET_DELAY = 50ms;


// Exceptions ----------------------------------------------------------------------------
struct NullPtr : std::exception {
	const char* what() const noexcept { return "PC_Network.cpp ERROR: Null Pointer Encountered.\n"; }
};
struct BuffSmall : std::exception {
	const char* what() const noexcept { return "PC_Network.cpp ERROR: Packet size exceeds BUFFER_SIZE.\n"; }
};
struct EmptyPack : std::exception {
	const char* what() const noexcept { return "PC_Network.cpp ERROR: Provided Packet is empty.\n"; }
};
struct IPStrNotNullTerm : std::exception {
	const char* what() const noexcept { return "PC_Network.cpp ERROR: IP Address String is not Null Terminated.\n"; }
};
struct InvalidIPAddr : std::exception {
	const char* what() const noexcept { return "PC_Network.cpp ERROR: IP Address String is not a valid IPv4 Address.\n"; }
};




// NPeer ---------------------------------------------------------------------------------
/* Member Implementation Documentation
 *  @member udp  Shared UDP socket used to send data out to all peers.  Setting
 *              this paremeter in any NPeer class sets it for all as per the
 *              static keyword.
 *
 * @member tcp  TCP Socket used to communicate data to this specific peer.  Used
 *              for data where guaranteed delivery is important... pretty much
 *              everything except for audio.
 *
 * @member udp_dest  Destination address for this peer's UDP socket. Audio
 *                   data sent over @udp will be sent to this address.
 *
 * @member in_packets  Priority queue of type @AudioInPacket ordered by
 *                     @packet_id such that popping off an element will get you
 *                     the lowest numbered packet.  This is used to get packets
 *                     in order even if they arrived out of order.
 *
 * @member in_queue_lock  Mutex lock to ensure mutual exclusion with @in_packets
 *
 * @member in_bucket  Queue of type @AudioInPacket.  It's used to store packets
 *                    that aren't currently in @in_packets.  The point of this
 *                    is to store the unused packets instead of destructing and
 *                    constructing them every time.
 *
 * @member in_bucket_lock  See @in_queue_lock but with @in_packets replaced with
 *                         @in_bucket.
 *
 * @member in_packet_id  @packet_id from the last @AudioInPacket that was
 *                       popped off @in_packets.  Used to identify if any
 *                       packets were dropped.
 *
 * @member out_packets  Queue of type @AudioOutPacket.  Exists for the purpose
 *                      of queuing up encoded audio to be sent over network to
 *                      peer.
 *
 * @member out_queue_lock  See @in_queue_lock but for @out_packets instead of
 *                         @in_packets.
 *
 * @member out_bucket  See @in_bucket but with @AudioOutPacket instead of
 *                     @AudioInPacket and @out_packets instead of @in_packets.
 *
 * @member out_packet_id  Next id that will be assigned to @packet_id in
 *                        @AudioPacket.  Handles sequentially numbering packets.
 *
 */
// Static Initialization
int NPeer::udp = -1;


// Constructor
NPeer::NPeer() noexcept : in_packets(AudioInPacket_greater)
{
	std::memset((void*)&(this->udp_dest), 0, sizeof(sockaddr_in));
	udp_dest.sin_family = AF_INET;
}


NPeer::NPeer(const char *ip, const uint16_t &port) : NPeer()
{
	if(!ip) throw NullPtr();
	for(int i = 0; ; ++i)
	{
		if(ip[i] == 0) break;
		else if(i == IP_LEN) throw IPStrNotNullTerm();
	}

	if(inet_pton(AF_INET, ip, &(this->udp_dest.sin_addr)) != 1) throw InvalidIPAddr();
	this->udp_dest.sin_port = htons(port);
}


NPeer::~NPeer() { }


// Sending Audio
AudioOutPacket* NPeer::getEmptyOutPacket() noexcept
{
	AudioOutPacket *packet;
	out_bucket_lock.lock();

	if(!out_bucket.empty())
	{
		packet = out_bucket.front().release();
		out_bucket.pop();
	}
	else
		packet = new AudioOutPacket;

	out_bucket_lock.unlock();
	return packet;
}


void NPeer::enqueue_out(AudioOutPacket * &packet)
{
	if(!packet) throw NullPtr();
	else if(packet->packet_len == 0) throw EmptyPack();

	packet->packet_id = out_packet_id++;

	out_queue_lock.lock();
	out_packets.emplace(packet);
	out_queue_lock.unlock();

	out_packet_count++;
	packet = NULL;
}


void NPeer::startNetStream() noexcept
{
	if(run_thread) return;
	run_thread = true;
	std::thread(&NPeer::send_audio_over_network_thread, this).detach();
}



void NPeer::stopNetStream() noexcept
{
	run_thread = false;
}


// Receiving Audio
AudioInPacket* NPeer::getEmptyInPacket() noexcept
{
	AudioInPacket *packet;
	in_bucket_lock.lock();

	if(!in_bucket.empty())
	{
		packet = in_bucket.front().release();
		in_bucket.pop();
	}
	else
		packet = new AudioInPacket;

	in_bucket_lock.unlock();
	return packet;
}


void NPeer::retireEmptyInPacket(AudioInPacket * &packet) noexcept
{
	in_bucket_lock.lock();
	in_bucket.emplace(packet);
	in_bucket_lock.unlock();

	packet = NULL;
}


void NPeer::enqueue_in(AudioInPacket * &packet)
{
	if(!packet) throw NullPtr();
	else if(packet->packet_len == 0) throw EmptyPack();

	packet->received = steady_clock::now();

	in_queue_lock.lock();
	in_packets.emplace(packet);
	in_queue_lock.unlock();

	packet = NULL;
}


AudioInPacket* NPeer::getAudioInPacket() noexcept
{
	AudioInPacket *packet = NULL;
	in_queue_lock.lock();

	if(!in_packets.empty())
	{
		packet = in_packets.top().get();
		if((steady_clock::now() - packet->received) > PACKET_DELAY)
		{
			const_cast<std::unique_ptr<AudioInPacket>&>(in_packets.top()).release();
			in_packets.pop();
		}
		else
			packet = NULL;
	}

	in_queue_lock.unlock();
	if(packet) in_packet_id = packet->packet_id;
	return packet;
}


bool NPeer::create_udp_socket() noexcept
{
	if (udp > 0) close(udp);
	udp = -1;
	if((udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("NPeer::create_udp_socket()");
		udp = -1;
		return false;
	}
	else return true;
}


/*
 * Get @AudioOutPacket that is populated with audio packet data.  The data should be
 * provided by the client audio processor from the microphone.
 *
 * @return  A pointer to an AudioOutPacket OR a NULL pointer if none are available.
 */
AudioOutPacket* NPeer::getAudioOutPacket() noexcept
{
	AudioOutPacket* packet = NULL;
	out_queue_lock.lock();

	if(!out_packets.empty())
	{
		packet = out_packets.front().release();
		out_packets.pop();
	}

	out_queue_lock.unlock();
	return packet;
}


/*
 * Retire an AudioOutPacket that isn't needed anymore so it can be recylced later.
 *
 * @param packet  A pointer to the AudioOutPacket.
 */
void NPeer::retireEmptyOutPacket(AudioOutPacket * &packet) noexcept
{
	out_bucket_lock.lock();
	out_bucket.emplace(packet);
	out_bucket_lock.unlock();

	packet = NULL;
}


void NPeer::send_audio_over_network_thread() noexcept
{
	static const int SENDV_SIZE = 9;
	uint8_t buffer[BUFFER_SIZE + SENDV_SIZE];
	union {
		uint32_t word;
		uint8_t  byte[4];
	} split;

	// Create UDP Server if not already
	if(udp < 0)
	{
		if(!create_udp_socket())
		{
			std::cerr << "NPeer ERROR: Could not create udp socket" << std::endl;
			run_thread = false;
		}
	}


	// Main While Loop to stay in function
	while(run_thread)
	{
		// Yield if no audio packets
		if(out_packet_count.load() <= 0)
		{
			std::this_thread::yield();
			continue;
		}

		// Retrieve Audio Packet and decrement counter
		AudioOutPacket *packet = getAudioOutPacket();
		out_packet_count--;

		// Copy into local buffer leaving room for meta data
		std::memcpy(buffer + SENDV_SIZE, packet->packet.get(), packet->packet_len);

		// Tag Type
		buffer[0] = SENDV;

		// Tag Packet ID
		split.word = htonl(packet->packet_id);
		buffer[1]  = split.byte[0];
		buffer[2]  = split.byte[1];
		buffer[3]  = split.byte[2];
		buffer[4]  = split.byte[3];

		// Tag Packet Content Length
		split.word = htonl(packet->packet_len);
		buffer[5]  = split.byte[0];
		buffer[6]  = split.byte[1];
		buffer[7]  = split.byte[2];
		buffer[8]  = split.byte[3];

		// Send
		packet->packet_len += SENDV_SIZE;
		__attribute__((unused)) ssize_t sent = sendto(udp, packet->packet.get(),
		                                              packet->packet_len, 0,
		                                              (const sockaddr*) &udp_dest, sizeof(udp_dest));
		#ifdef DEBUG
		if(sent != packet->packet_len)
		{
			std::cerr << "NPeer Audio Out Thread WARNING: Bytes Sent( " << sent << ") != Bytes Intended(" << packet->packet_len << ")\n";
		}
		#endif
	}
}


