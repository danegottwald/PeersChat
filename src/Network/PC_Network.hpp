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

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>

#elif   _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#elif  __APPLE__

// Lmao

#endif


// Universal Includes
#include <cstdint>
#include <cstring>
#include <vector>
#include <queue>
#include <functional>
#include <memory>
#include <mutex>
#include <exception>


// Pre-Compiler Constants
#define BUFFER_SIZE 4096


// Globals
extern std::chrono::milliseconds PACKET_DELAY; //defaults to 50ms



// Audio Packet Struct -------------------------------------------------------------------
/* AudioPacket: A struct that contains an encoded audio packet
 *
 * @member packet_id  Unique id for an incoming packet from this peer.  Packets
 *                    received will be numbered sequentially starting with zero
 *
 * @member packet_len The size of packet in bytes
 *
 * @member packet  A unique pointer to a buffer meant for containing an encoded
 *                 audio packet from libopus
 *
 * @constructor AudioPacket()  Initializes packet to at least BUFFER_SIZE which
 *                             is always at least 4096
 *
 * @constructor AudioPacket(2)  Same as @AudioPacket() but allows you to
 *                              populate @packet and set @packet_len.
 *
 * @operator <  Less than operator overload for comparing and sorting based
 *              packet id number.
 */
struct AudioPacket
{
	uint32_t packet_id = 0;
	uint16_t packet_len = 0;
	std::unique_ptr<uint8_t> packet = std::unique_ptr<uint8_t>(new uint8_t[BUFFER_SIZE]);
	inline bool operator<(const AudioPacket &other) { return this->packet_id < other.packet_id; }
};

	// Type change for static error checking
struct AudioInPacket  : public AudioPacket { std::chrono::time_point<std::chrono::steady_clock> received; };
struct AudioOutPacket : public AudioPacket { };

	// Comparator for std::priority_queue that pops the smallest packet first
inline bool AudioInPacket_greater(const std::unique_ptr<AudioInPacket> &left, const std::unique_ptr<AudioInPacket> &right) { return !((*left) < (*right)); }


// NPeer Class ---------------------------------------------------------------------------
/* NPeer: A class for handling networking to your peers -- API DOCUMENTATION
 *
 * @constructor NPeer(0)  Default constructor
 *
 * @constructor NPeer(2)  Constructor that initializes address structure for peer
 *                        destination.
 *                      @param ip: (const char*)  IP address in standard string format
 *                                   Ex: 192.168.1.120 or 127.0.0.1 or ...
 *                      @param port: (const uint16_t)  Destination port number in host
 *                              byte order
 *                              Ex: 8080
 *
 * @method getEmptyOutPacket()  Method that returns an @AudioOutPacket.  Packet may have
 *                              junk/old data in it, make sure to overwrite.
 *                              @AudioOutPacket returned should be passed to @enqueue_out
 *                              after being populated with audio packet data.
 *                            @return (AudioOutPacket*) Pointer to AudioOutPacket that is
 *                              given to the client to be populated with audio/mic data.
 *
 * @method enqueue_out(1)  Enqueue's encoded audio packet in the form of @AudioOutPacket
 *                         for the purposes of being sent to peer through @udp socket
 *                       @param packet: (AudioOutPacket*)  A pointer to an AudioOutPacket
 *                               that is populated with audio data from client.
 *
 * @method getEmptyInPacket()  Method that returns a @AudioInPacket.  Packet may contain
 *                             junk data that should be overwritten.  Should be populated
 *                             with mic data from peer.
 *                           @return (AudioInPacket*) Pointer to AudioInPacket that is
 *                            given to the client to be populated with audio from peer.
 *
 * @method retireEmptyInPacket(1)  Method that retires old/used/processed @AudioInPacket
 *                                 to be recycled later.
 *                               @param packet (AudioInPacket*) Pointer to AudioInPacket
 *                                       that is to be retired.
 *
 * @method enqueue_in(1)  Enqueue's an audio packet.  Get's queue'd into @in_packets
 *                        Priority Queue so the lowest id packet gets popped first.
 *                      @param packet (AudioInPacket*) Pointer to packet that is populated
 *                              with peer mic data
 *
 * @method getAudioInPacket()  Get @AudioInPacket that is populated with audio packet
 *                             data.  Data comes from peer microphone.  packet_id from
 *                             return struct can be used in conjunction with
 *                             @getInPacketId to identify packet loss scenarios
 *                           @return (AudioInPacket*) Pointer to AudioInPacket populated
 *                             with audio from peer
 *
 * @method getInPacketId()  Returns what the id of the last AudioInPacket was.  Should
 *                          be used to identify packet loss.  Call this to get packet_id
 *                          then call @getAudioInPacket.  The difference between the two
 *                          id's provided by those functions should be 1 when there is no
 *                          packet loss, and if there is packet loss, the number of
 *                          lost packets will be the difference minus 1.
 *                        @return (uint32_t) Last packet id returned by @getAudioInPacket
 *                          Ex: uint32_t lastId = getInPacketId();
 *                              AudioInPacket *packet = getAudioInPacket();
 *                              int packets_lost = packet->packet_id - lastId - 1;
 *
 */
class NPeer
{
	// Members
private:
		// UDP Socket for audio to/from everyone | TCP Socket for comms to this Peer
	static int udp;
	int tcp = -1;
		// Peer Address
	sockaddr_in udp_dest;
		// Audio Incoming
	std::priority_queue<std::unique_ptr<AudioInPacket>,
	                    std::vector<std::unique_ptr<AudioInPacket>>,
	                    std::function<bool(std::unique_ptr<AudioInPacket>&, std::unique_ptr<AudioInPacket>&)>> in_packets;
	std::mutex in_queue_lock;
	std::queue<std::unique_ptr<AudioInPacket>> in_bucket;
	std::mutex in_bucket_lock;
	uint32_t in_packet_id = 0;
		// Audio Outgoing
	std::queue<std::unique_ptr<AudioOutPacket>> out_packets;
	std::mutex out_queue_lock;
	std::queue<std::unique_ptr<AudioOutPacket>> out_bucket;
	std::mutex out_bucket_lock;
	uint32_t out_packet_id = 0;

	// Constructor
public:
	NPeer();
	NPeer(const char* ip, const uint16_t &port);
	~NPeer();

	// Sending Audio -- All the functions you need to send audio
	AudioOutPacket* getEmptyOutPacket();
	void enqueue_out(AudioOutPacket* packet);

	// Receiving Audio -- All the functions you need to receive audio
	AudioInPacket* getEmptyInPacket();
	void retireEmptyInPacket(AudioInPacket* packet);
	void enqueue_in(AudioInPacket* packet);
	AudioInPacket* getAudioInPacket();
	inline uint32_t getInPacketId() { return in_packet_id; }

	// Outgoing Audio Network Thread w/ Sending Audio Functions
private:
	AudioOutPacket* getAudioOutPacket();
	void retireEmptyOutPacket(AudioOutPacket* packet);
};


#endif

