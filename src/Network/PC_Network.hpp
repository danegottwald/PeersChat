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



// Audio Packet Struct ---------------------------------------------------------
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


// NPeer Class -----------------------------------------------------------------
/* NPeer: A class for handling networking to your peers
 *
 * @member udp  Shared UDP socket used to send data out to all peers.  Setting
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
 * @constructor NPeer(0)  Default constructor
 *
 * @constructor NPeer(2)  Constructor that initializes address structure for
 *                        peer destination.
 *
 * @method getEmptyOutPacket()  Method that returns an @AudioOutPacket.  Packet
 *                              may have junk/old data in it.  Make sure to
 *                              overwrite.  @AudioOutPacket returned should be
 *                              passed to enqueue_out after populated with
 *                              being packet data.
 *
 * @method enqueue_out(1)  Enqueue's encoded audio packet in the form of
 *                         @AudioOutPacket for the purposes of being sent to
 *                         peer out @udp socket.
 *
 * @method getEmptyInPacket()  Method that returns an @AudioInPacket.  Packet
 *                             contains an audio packet to be played out
 *                             speaker.  @packet_id parameter in
 *                             @AudioInPacket can be used to identify how many
 *                             packets were dropped beforehand.
 *
 * @method retireEmptyInPacket(1)  Method that retires old/used/processed
 *                                 @AudioInPacket structs back into @in_bucket
 *                                 so they can be used later.
 *
 * @method enqueue_in(1)  Enqueue's an audio packet.  Get's queue'd into
 *                        @in_packets Priority Queue so the lowest id packet
 *                        gets popped first.
 *
 * @method getAudioInPacket()  Get @AudioInPacket that is populated with audio
 *                             packet data.  Data comes from peer microphone.
 *
 * @method getAudioOutPacket()  Get @AudioOutPacket that is populated with audio
 *                              packet data.  Data comes from client microphone.
 *
 * @method retireEmptyOutPacket(1)  See @retireEmptyInPacket, but for
 *                                  @AudioOutPacket and @out_bucket
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
	inline uint32_t getInPacketId() { return in_packet_id; }
	AudioInPacket* getEmptyInPacket();
	void retireEmptyInPacket(AudioInPacket* packet);
	void enqueue_in(AudioInPacket* packet);
	AudioInPacket* getAudioInPacket();

	// Outgoing Audio Network Thread w/ Sending Audio Functions
private:
	AudioOutPacket* getAudioOutPacket();
	void retireEmptyOutPacket(AudioOutPacket* packet);
};


#endif

