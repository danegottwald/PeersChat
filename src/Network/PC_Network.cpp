// This Library was written in its entirety by Omar Ahmadyar (oahmadyar@gmail.com)
#include "PC_Network.hpp"


// Namespace -----------------------------------------------------------------------------
using namespace std::chrono;
using namespace std::chrono_literals;


// Pre-Compiler Constants ----------------------------------------------------------------
#define IP_LEN 15


// Globals -------------------------------------------------------------------------------
std::chrono::milliseconds PACKET_DELAY = 50ms;
std::chrono::milliseconds PEERS_CHAT_NETWORK_DESTRUCT_TIMEOUT = 1000ms;


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
struct InvalidRecvAmount : std::exception {
	const char* what() const noexcept { return "PC_Network.cpp ERROR: Invalid Read Amount.\n"; }
};
struct InvalidResponse : std::exception {
	const char* what() const noexcept { return "PC_Network.cpp ERROR: Invalid Response.\n"; }
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
 * @member destination  Destination address for this peer's UDP socket. Audio
 *                      data sent over @udp will be sent to this address.
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
	std::memset((void*)&(this->destination), 0, sizeof(sockaddr_in));
	destination.sin_family = AF_INET;
}


NPeer::NPeer(const char *ip, const uint16_t &port) : NPeer()
{
	if(!ip) throw NullPtr();
	for(int i = 0; ; ++i)
	{
		if(ip[i] == 0) break;
		else if(i == IP_LEN) throw IPStrNotNullTerm();
	}

	if(inet_pton(AF_INET, ip, &(this->destination.sin_addr)) != 1) throw InvalidIPAddr();
	this->destination.sin_port = htons(port);
}


Npeer(sockaddr_in *addr)
{
	//TODO
}


NPeer::~NPeer()
{
	run_thread = false;
	try { audio_out_thread.join(); }
	catch(...) { }
}


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
	audio_out_thread = std::thread(&NPeer::send_audio_over_network_thread, this);
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

	do
	{
		if(packet) retireEmptyInPacket(packet);
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
	} while (packet && (packet->packet_id < in_packet_id));

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
		                                              (const sockaddr*) &destination, sizeof(destination));
		#ifdef DEBUG
		if(sent != packet->packet_len)
		{
			std::cerr << "NPeer Audio Out Thread WARNING: Bytes Sent( " << sent << ") != Bytes Intended(" << packet->packet_len << ")\n";
		}
		#endif
	}
}


bool NPeer::createTCP(sockaddr_in *addr)
{
	//TODO
}


void NPeer::destroyTCP()
{
	//TODO
}


// PeersChatNetwork Definitions ----------------------------------------------------------
// Static Helpers
static ssize_t send_timeout(int sockfd, const void *buf, size_t len, int flags)
{
	//TODO
}


static ssize_t recv_timeout(int sockfd, void *buf, size_t len, int flags);
{
	//TODO
}


// Constructors
PeersChatNetwork::PeersChatNetwork()
{
	peers.reserve(128);
}


PeersChatNetwork::~PeersChatNetwork()
{
	this->stop();
}


// Operators
NPeer* operator[](sockaddr_in *addr)
{
	//TODO
}


// Public Functions
bool join(sockaddr_in *addr)
{
	//TODO
}


int host()
{
	//TODO
}


void PeersChatNetwork::start()
{
	run_listen_thread = true;
	listen_thread.reset(new std::thread(&PeersChatNetwork::listen_on_tcp_thread, this));
}


void PeersChatNetwork::stop()
{
	run_listen_thread = false;
	for(std::unique_ptr<NPeer> &nptr : this->peers)
		nptr->stopNetStream();
	std::this_thread::sleep_for(PEERS_CHAT_DESTRUCT_TIMEOUT);
	listen_thread.reset();
}


bool PeersChatNetwork::propose(sockaddr_in *subject, int sock)
{
	// Tag Type of Request
	uint8_t buffer[7];
	buffer[0] = PROPOSE;

	// Attach IP Address of subject
	union {
		uint32_t addr;
		uint8_t  byte[4];
	} ip;
	ip.addr = subject->sin_addr.s_addr;
	buffer[1] = ip.byte[0];
	buffer[2] = ip.byte[1];
	buffer[3] = ip.byte[2];
	buffer[4] = ip.byte[3];

	// Attach Port Num of subject
	union {
		uint16_t num;
		uint8_t  byte[2];
	} port;
	port.num = subject->sin_port;
	buffer[5] = port.byte[0];
	buffer[6] = port.byte[1];

	// Send Proposal
	if(7 != send_timeout(sock, buffer, 7, MSG_NOSIGNAL))
		return false;
	else return true;
}


bool respond(bool decision, int sock)
{
	uint8_t buff;
	if(decision) buff = ACCEPT;
	else buff = DENY;

	if(1 != send_timeout(sock, &buff, 1, MSG_NOSIGNAL))
		return false;
	else return true;
}


bool getResponse(int sock)
{
	uint8_t buff;
	if(1 != recv_timeout(sock, &buff, 1, MSG_WAITALL))
		throw InvalidRecvAmount();
	if (buff == ACCEPT)
		return true;
	else if (buff == DENY)
		return false;
	else
		throw InvalidResponse();
}


bool PeersChatNetwork::addPeer(sockaddr_in *addr)
{
	this->peers.emplace_back(new NPeer(addr));
	((*this)[addr])->startNPeer();
}


bool PeersChatNetwork::requestPeers(int sock, std::vector<sockaddr_in> &peers)
{
	uint8_t buffer[BUFFER_SIZE];

	// Send REQP
	buffer[0] = REQP;
	ssize_t s = send_timeout(sock, buffer, 1, MSG_NOSIGNAL);
	if(1 != s) return false;

	// Receive SENDP and Content Length
	s = recv_timeout(sock, buffer, 5, MSG_WAITALL);
	if(s != 5) return false;
	if(buffer[0] != SENDP) return false;
	uint32_t content_length = (buffer[1] << 24) | (buffer[2] << 16) | (buffer[3] << 8) | (buffer[4]);

	// Loop Over Addresses & Add to Vector
	auto min = [](const ssize_t &a, const ssize_t &b) { return (a<b)?a:b; }
	union { uint16_t num; uint8_t byte[2]; } port;
	union { uint32_t num; uint8_t byte[4]; } ip;
	sockaddr_in addr;
	ssize_t total = 0;
	while(total < content_length)
	{
		// Receive Data
		s = recv_timeout(sock, buffer, min(BUFFER_SIZE, content_length-total), MSG_WAITALL);
		if (s < 0) return false;
		else if(s % 6 != 0) return false;
		total += s;

		// Add each address to vector
		for(ssize_t pos = 0; pos < s; pos += 6)
		{
			port.byte[0] = buffer[pos + 0];
			port.byte[1] = buffer[pos + 1];
			port.byte[2] = buffer[pos + 2];
			port.byte[3] = buffer[pos + 3];
			in.byte[0]   = buffer[pos + 4];
			in.byte[1]   = buffer[pos + 5];
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = port.num;
			addr.sin_port = ip.num;
			peers.push_back(addr);
		}
	}
	return true;
}


void PeersChatNetwork::sendPeers(int sock)
{
	uint8_t buffer[BUFFER_SIZE];

	// Tag Type
	buffer[0] = SENDP;

	// Unions
	union { uint32_t num; uint8_t byte[4]; } word;
	union { uint16_t num; uint8_t byte[2]; } port;

	// Add Content Length
	uint32_t content_length = this->peers.size() * 6;
	word.num = htonl(content_length);
	buffer[1] = word.num[0];
	buffer[2] = word.num[1];
	buffer[3] = word.num[2];
	buffer[4] = word.num[3];

	// Populate With Peers
	uint32_t pos = 5;
	for(std::unique_ptr<NPeer> &ptr : this->peers)
	{
		sockaddr_in addr = NPeerAttorney::getDest(ptr.get());
		word.num = addr.sin_addr.s_addr;
		port.num = addr.sin_port;
		buffer[pos + 0] = word.byte[0];
		buffer[pos + 1] = word.byte[1];
		buffer[pos + 2] = word.byte[2];
		buffer[pos + 3] = word.byte[3];
		buffer[pos + 4] = port.byte[0];
		buffer[pos + 5] = port.byte[1];
		pos += 6;
	}
	ssize_t size = send_timeout(sock, buffer, content_length + 5, MSG_NOSIGNAL);
}


void PeersChatNetwork::disconnect()
{
	//TODO
}


void PeersChatNetwork::receive_audio_thread()
{
	uint8_t buffer[BUFFER_SIZE];

	ssize_t r = 0;
	sockaddr_in addr;
	while(run_listen_thread)
	{
		memset(addr, 0, sizeof(addr));

		// Receive Packet
		r = recvfrom(NPeerAttorney::getUDP(), buffer, BUFFER_SIZE,
		             MSG_WAITALL, (sockaddr*) &addr, sizeof(addr));
		if(r < 0) continue;
		if(buffer[0] != SENDV) continue;

		// Sort
		NPeer *peer = (*this)[addr];

		// Get Empty Packet
		AudioInPacket *pack = peer->getEmptyInPacket();

		// Set Packet ID, Packet Length, and Copy Data
		pack->packet_id  = (buffer[1] << 24) | (buffer[2] << 16) | (buffer[3] << 8) | (buffer[4]);
		pack->packet_len = (buffer[5] << 24) | (buffer[6] << 16) | (buffer[7] << 8) | (buffer[8]);
		std::memcpy(pack->packet.get(), buffer + 9, pack->packet_len);

		// Queue
		peer->enqueue_in(pack);
	}
}


void PeersChatNetwork::listen_on_tcp_thread()
{
	//TODO
}


// This Library was written in its entirety by Omar Ahmadyar (oahmadyar@gmail.com)
