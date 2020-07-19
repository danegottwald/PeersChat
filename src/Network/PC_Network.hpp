// This Library was written in its entirety by Omar Ahmadyar (oahmadyar@gmail.com).

#ifndef _PC_NETWORK_HPP
#define _PC_NETWORK_HPP




/*
 *  PeersChat Networking Header: A Library to handle all PeersChat Networking
 *
 * There are a few different classes/structs to be aware of in this Library:
 *
 *    AudioPacket: A struct that is used to pass around AudioPackets in buffers.
 *
 *      AudioInPacket/AudioOutPacket are derivative child structs that inherit from
 *      AudioPacket.  Don't concern yourself with their differences, the main reason
 *      for the difference in name is to cause a compiler error when you mix them
 *
 *    NPeer: A class used to maintain communications with a single peer.  Contains several
 *           Queue's to handle Packets going in and out over network.  Ensures that
 *           you don't get packets out of order.  Handles sending audio for you.  Will
 *           provide you with audio as long as some producer is retrieving them from
 *           network.
 *
 *    PeersChatNetwork: A class used to handle all networking in PeersChat.  Contains
 *                      some amount of NPeer objects dependent upon how many people you
 *                      are connected to.  Will handle people joining/leaving a call and
 *                      other things.  Will retrieve audio packets from the network and
 *                      pass them off to their respective NPeer objects based on who sent
 *                      the packet.
 *
 *
 *
 * How to use this Library as:
 *    An Audio Encoder Library:  Whenever you have encoded audio you want to send, go
 *                               ahead and access the NPeer objects in the
 *                               PeersChatNetworking object and request empty
 *                               AudioOutPacket's from each of them.  Then fill it with
 *                               data and enqueue_out them.
 *                               Whenever you want to retrieve encoded audio from your
 *                               peers, loop over all the NPeers in the PeersChatNetwork
 *                               object and getAudioInPacket from all of them.  You can
 *                               merge and decode the audio and queue it up to play out
 *                               the speaker.  Whenever that's done, recycle the now
 *                               processed AudioInPackets so they can be recirculated.
 *
 *   A GUI/Main Thread:  Whatever requests you have you can link to the public functions
 *                       made available to you through the PeersChatNetwork class.  Join,
 *                       host, etc. by calling the respective functions.
 *
 *
 */




/*
 * This library is initially written for GNU/Linux systems
 * Includes will have to be moved in/out of guards for cross-platform support
 */
#ifdef __linux__

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#elif   _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
typedef ssize_t  int64_t;
typedef size_t  uint64_t;
static inline int close(int &x) { return closesocket(x); }

#elif  __APPLE__

// Lmao

#endif


// Debug
#ifdef DEBUG
	#define NET_DEBUG
#endif


// Universal Includes
#include <iostream>
#include <cstdint>
#include <cinttypes>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <queue>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include <exception>
#include <thread>
#include <stdio.h>
#include <errno.h>
#include "nettypes.hpp"


// Pre-Compiler Constants
#define BUFFER_SIZE 4096
#define MAX_NAME_LEN 18
#define MAX_PEERS 64


// Globals
	// Delay to give packets time to catch up and sort
extern std::chrono::milliseconds PACKET_DELAY;
	// Time to give threads time to end before forcing them to
extern std::chrono::milliseconds PEERS_CHAT_DESTRUCT_TIMEOUT;
	// How long it takes for a send/recv to timeout
extern std::chrono::milliseconds SOCKET_TIMEOUT;
	// Time to give PEER to send you audio before they timeout
extern std::chrono::milliseconds PEER_TIMEOUT;
	// Port Number to handle UDP/TCP
extern uint16_t PORT;



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
struct AudioInPacket  : public AudioPacket {
private:
	std::chrono::time_point<std::chrono::steady_clock> received;
	friend class NPeer;
};
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
 * @operator ==(1)  Equivalence operator.  Compare an address to see if this NPeer routes
 *                  to that address.
 *                 @param addr (sockaddr_in)  Adress you want to check equivalence with
 *                 @return (bool) true if the addresses match, false otherwise
 *
 */
class NPeer
{
	// Members
private:
	int tcp = -1;
	static int udp;
	sockaddr_in destination;
	char pname[MAX_NAME_LEN+1];
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
	bool run_thread = false;
	std::atomic<int> out_packet_count = {0};

	// Constructor
public:
	NPeer() noexcept;
	NPeer(const char* ip, const uint16_t &port);
	NPeer(const sockaddr_in &addr) noexcept;
	~NPeer() noexcept;

	// Name
	std::string getName() noexcept;
	bool setName(std::string name) noexcept;


	// Sending Audio -- All the functions you need to send audio
	AudioOutPacket* getEmptyOutPacket() noexcept;
	void enqueue_out(AudioOutPacket * &packet);
	void startNetStream() noexcept;
	void stopNetStream() noexcept;

	// Receiving Audio -- All the functions you need to receive audio
	AudioInPacket* getEmptyInPacket() noexcept;
	void retireEmptyInPacket(AudioInPacket * &packet) noexcept;
	void enqueue_in(AudioInPacket *packet);
	AudioInPacket* getAudioInPacket() noexcept;
	inline uint32_t getInPacketId() noexcept { return in_packet_id; }

	// Equivalence Operator
	bool operator==(const sockaddr_in &addr) noexcept;

	// Outgoing Audio Network Thread w/ Sending Audio Functions
private:
	std::unique_ptr<std::thread> audio_out_thread;
	static bool create_udp_socket() noexcept;
	AudioOutPacket* getAudioOutPacket() noexcept;
	void retireEmptyOutPacket(AudioOutPacket * &packet) noexcept;
	void send_audio_over_network_thread() noexcept;

	// Connections over TCP
	bool createTCP();
	void destroyTCP();
	inline sockaddr_in getDest() { return destination; }
	friend class NPeerAttorney;
};


class NPeerAttorney
{
	static inline bool createTCP(NPeer *peer) {
		return peer->createTCP();
	}
	static inline void destroyTCP(NPeer *peer) {
		peer->destroyTCP();
	}
	static inline sockaddr_in getDest(NPeer *peer) {
		return peer->getDest();
	}
	static inline int getUDP() { return NPeer::udp; }
	static inline int getTCP(NPeer *peer) { return peer->tcp; }
	friend class PeersChatNetwork;
};


// PeersChatNetwork Class ----------------------------------------------------------------
/* PeersChatNetwork: A Class that handles all your PeersChat networking needs
 *
Members:
 * peers  Vector of NPeers that we are in a call with
 *
 * tcp_listen  Socket we are listening for tcp requests on
 * accept_direct_join  Flag that indicates whether we are going to allow people to join
 *                     the call through this client.
 *
 * accept_indirect_join  Flag that indicates whether we are going to allow anybody to join
 *                       the call at all
 *
 * running  Flag that indicates whether PeersChatNetwork is currently running
 *
 *
Constructors:
 * PeersChatNetwork()  Create PeersChatNetwork Object
 * ~PeersChatNetwork()  Destroy PeersChatNetwork Object
 *
Operators:
 * operator[sockaddr_in]  Return the a pointer to the NPeer object corresonpding to a
 *                        specific destination address
 *
 * operator[int]  Return the pointer to the Npeer object stored at position x.  Useful
 *                for looping through all NPeer objects.
 *
 *
Public Methods:
 * join(sockaddr_in)  Join a PeersChat session using a sockaddr_in struct as the dest
 *
 * host()  Host your own PeersChat session
 *
 * start()  Start Receiving Audio and listening for connections
 *
 * stop()  Stop Receiving Audio and listening for connections
 *
 * getNumberPeers()  Get number of other peers.  Useful for looping over them.
 *
 *
Private Methods:
 * propose(sockaddr_in, int)  Ask currently connected peer(int) if person represented
 *                            by sockaddr_in can join
 *
 * respond(bool, int)  ACCEPT/DECLINE currently connected peer's(int) last statement
 *
 * getResponse(int)  Find out if they accepted or declined your request
 *
 * addPeer(sockaddr_in)  Add new member to your PeersChatNetwork as an NPeer
 *
 * requestPeers(int, std::vector<sockaddr_in>)  Ask for peer's entire peer list
 *
 * sendPeers(int) Send all your peers to peer at socket(int)
 *
 * connect(int)  Ask to connect to client at socket(int)
 *
 * disconnect(int)  Tell peer at socket(int) you are disconnecting
 *
 * receive_audio_thread  Function to start in its own thread to receive audio
 *
 * listen_on_tcp_thread  Function to start in its own thread to listen for any
 *                       tcp connections/requests.
 *
 */
class PeersChatNetwork
{
	// Members
private:
	char myname[MAX_NAME_LEN+1] = {0};
	std::vector<std::unique_ptr<NPeer>> peers;
	std::mutex peers_lock;
	int size = 0;
	int tcp_listen = -1;
	bool accept_direct_join = false;
	bool accept_indirect_join = true;
	bool running = false;
	std::unique_ptr<std::thread> listen_thread;
	std::unique_ptr<std::thread> recv_thread;
	char myName[MAX_NAME_LEN + 1] = {0};

public:
	PeersChatNetwork();
	~PeersChatNetwork();

	NPeer* operator[](const sockaddr_in &addr) noexcept;
	NPeer* operator[](const int &x) noexcept;
	NPeer* operator[](const std::string &x) noexcept;

	std::string getMyName() noexcept;
	bool setMyName(const std::string&) noexcept;

	bool join(const sockaddr_in &addr) noexcept;
	void getNames() noexcept;
	bool host() noexcept;
	void disconnect() noexcept;
	inline int getNumberPeers() { return this->size; }

private:
	bool start() noexcept;
	void stop() noexcept;
	bool propose(sockaddr_in &subject, int sock) noexcept;
	bool respond(bool decision, int sock) noexcept;
	bool getResponse(int sock) noexcept;
	bool addPeer(const sockaddr_in &addr) noexcept; //new person joining group, add them
	void removePeer(sockaddr_in &addr) noexcept;
	bool requestPeers(int sock, std::vector<sockaddr_in>& provide_empty_vector) noexcept;
	void sendPeers(int sock);
	void connect(int sock);
	void disconnect(int sock);
	std::string getName(int sock) noexcept;

	void receive_audio_thread(); //thread that receives audio
	void listen_on_tcp_thread();

	bool connectFulfill(int sock, sockaddr_in addr);
	bool proposeFulfill(int sock, sockaddr_in addr);
};


#endif
// This Library was written in its entirety by Omar Ahmadyar (oahmadyar@gmail.com).

