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
#include <queue>
#include <vector>
#include <functional>
#include <memory>




// Audio Packet Struct ---------------------------------------------------------
/* AudioPacket: A struct that contains info 
 *
 */
struct AudioPacket
{
	uint64_t packet_id;
	uint16_t packet_len;
	std::unique_ptr<uint8_t> packet;

	AudioPacket();
	inline bool operator<(const AudioPacket &other) { return this->packet_id < other.packet_id; }
};


// NPeer Class -----------------------------------------------------------------
/* NPeer: A class for handling networking to your peers
 *
 * TODO: Documentation
 */
class NPeer
{
	// Static Member
private:
	static int socket;

	// Members
private:
	struct sockaddr_in;
	std::priority_queue<AudioPacket> packets;

	// Constructor
public:
	NPeer();
	NPeer(char* ip, const int &PORT);

	// Send
public:

	// Queue Packet
public:

};


#endif

