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




// AudioPacket Definitions ---------------------------------------------------------------
AudioPacket::AudioPacket() : packet_id(0), packet_len(0), packet(new uint8_t[BUFFER_SIZE]) { }


AudioPacket::AudioPacket(uint8_t *packet, size_t packet_len) : AudioPacket()
{
	this->packet_len = packet_len;
	if(!packet) throw NullPtr();
	else if(packet_len > BUFFER_SIZE) throw BuffSmall();
	std::memcpy(this->packet.get(), packet, packet_len);
}


// NPeer Definitions ---------------------------------------------------------------------
// Constructor
NPeer::NPeer() : tcp(-1), in_packets(AudioInPacket_greater), in_packet_id(0), out_packet_id(0)
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
AudioOutPacket* NPeer::getEmptyOutPacket()
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


void NPeer::enqueue_out(AudioOutPacket* packet)
{
	if(!packet) throw NullPtr();
	else if(packet->packet_len == 0) throw EmptyPack();

	packet->packet_id = out_packet_id++;

	out_queue_lock.lock();
	out_packets.emplace(packet);
	out_queue_lock.unlock();
}


// Receiving Audio
AudioInPacket* NPeer::getEmptyInPacket()
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


void NPeer::retireEmptyInPacket(AudioInPacket *packet)
{
	in_bucket_lock.lock();
	in_bucket.emplace(packet);
	in_bucket_lock.unlock();
}


void NPeer::enqueue_in(AudioInPacket* packet)
{
	if(!packet) throw NullPtr();
	else if(packet->packet_len == 0) throw EmptyPack();

	packet->received = steady_clock::now();

	in_queue_lock.lock();
	in_packets.emplace(packet);
	in_queue_lock.unlock();
}


AudioInPacket* NPeer::getAudioInPacket()
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
	out_packet_id = packet->packet_id;
	return packet;
}


// Outgoing Audio Network Thread w/ Sending Audio Functions
AudioOutPacket* NPeer::getAudioOutPacket()
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


void NPeer::retireEmptyOutPacket(AudioOutPacket *packet)
{
	out_bucket_lock.lock();
	out_bucket.emplace(packet);
	out_bucket_lock.unlock();
}


