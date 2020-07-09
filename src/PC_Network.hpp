#ifndef _PC_NETWORK_HPP
#define _PC_NETWORK_HPP




/*
 *  PeersChat Networking Header
 *  TODO: Documentation
 */




/*
 * TODO: Modify this section for OS compatibility
 * This library is initially written for GNU/Linux systems
 * Includes will have to be moved in/out of guards for cross-platform support
 */
#ifdef __linux__

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

#elif   _WIN32

#include <winsock2.h>

#elif  __APPLE__

// Lmao

#endif


// Universal Includes
#include <cstdint>
#include <vector>
#include <queue>
#include <functional>
#include <memory>
#include <mutex>




// Audio Packet Struct ---------------------------------------------------------
/* AudioPacket: A struct that contains an encoded audio packet
 *
 */
struct AudioPacket
{
	uint32_t packet_id;
	uint16_t packet_len;
	std::unique_ptr<uint8_t> packet;

	AudioPacket();
	inline bool operator<(const AudioPacket &other) { return this->packet_id < other.packet_id; }
};

	// Type change for static error checking
struct AudioInPacket  : public AudioPacket { };
struct AudioOutPacket : public AudioPacket { };


// NPeer Class -----------------------------------------------------------------
/* NPeer: A class for handling networking to your peers
 *
 * TODO: Documentation
 */
class NPeer
{
	// Members
private:
		// UDP Socket for audio to/from everyone | TCP Socket for comms to this Peer
	static int udp;
	int tcp;
		// Peer Address
	struct sockaddr_in;
		// Audio in
	std::priority_queue<std::unique_ptr<AudioInPacket>> in_packets;
	std::mutex in_queue_lock;
	std::queue<std::unique_ptr<AudioInPacket>> in_bucket;
	std::mutex in_bucket_lock;
	uint32_t in_packet_id;
		// Audio out
	std::queue<std::unique_ptr<AudioOutPacket>> out_packets;
	std::mutex out_queue_lock;
	std::queue<std::unique_ptr<AudioOutPacket>> bucket;
	std::mutex out_bucket_lock;
	uint32_t out_packet_id;

	// Constructor
public:
	NPeer();
	NPeer(char* ip, const int &PORT);

	// Sending Audio
	AudioOutPacket* getEmptyOutPacket();
	void enqueue_out(AudioOutPacket* packet);

	// Receiving Audio
	AudioInPacket* getEmptyInPacket();
	void enqueue_in(AudioInPacket* packet);
};


#endif

