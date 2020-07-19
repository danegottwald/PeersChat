// This Library was written in its entirety by Omar Ahmadyar (oahmadyar@gmail.com)
#include "PC_Network.hpp"


// Namespace -----------------------------------------------------------------------------
using namespace std::chrono;
using namespace std::chrono_literals;


// Pre-Compiler Constants ----------------------------------------------------------------
#define IN_PACKET_BUFFER_TOO_LARGE 10


// Globals -------------------------------------------------------------------------------
std::chrono::milliseconds PACKET_DELAY = 50ms;
std::chrono::milliseconds PEERS_CHAT_DESTRUCT_TIMEOUT = 2s;
std::chrono::milliseconds SOCKET_TIMEOUT = 5s;
std::chrono::milliseconds PEER_TIMEOUT = 15s;
uint16_t PORT = 8080;


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
struct TCPServerCreateFail : std::exception {
	const char* what() const noexcept { return "PC_Network.cpp ERROR: Could not create TCP Server Socket.\n"; }
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
 * @member run_thread  (bool) True if you want the audio out net stream thread to run
 *
 * @member out_packet_count  The number of outgoing packet's queue'd for delivery
 *
 * @member audio_out_thread  std::thread for the audio_out_thread that handles sending
 *                           audio packets
 *
 * @method (static) create_udp_socket  Creates/initializes a udp socket
 *
 * @method getAudioOutPacket  Get AudioOutPacket that is populated with audio packet data.
 *                            The data should be provided by the client audio processor
 *                            from the microphone.
 *                           @return A pointer to an AudioOutPacket or NULL pointer
 *
 * @method retireEmptyOutPacket  Retire an AudioOutPacket that isn't needed anymore so
 *                               it can be recylced later.
 *                              @param packet  A pointer to the AudioOutPacket
 *
 * @method send_audio_over_network_thread  Function designed to run on its own thread
 *                                         to handle the sending of audio.
 *
 * @method createTCP  Create a TCP connection to this specific NPeer
 *                   @return (bool) True if the operation was successful
 *
 * @method getDest()  Returns sockaddr_in struct that represents NPeer address
 *
 */
// Static Initialization
int NPeer::udp = -1;


// Constructor
NPeer::NPeer() noexcept : in_packets(AudioInPacket_greater)
{
	this->pname[0] = 0;
	std::memset((void*)&(this->destination), 0, sizeof(sockaddr_in));
	destination.sin_family = AF_INET;
	if(udp < 0)
		if(!create_udp_socket())
			exit(24);
}


NPeer::NPeer(const char *ip, const uint16_t &port) : NPeer()
{
	if(!ip) throw NullPtr();
	for(int i = 0; ip[i] != 0; ++i)
		if(i == INET_ADDRSTRLEN) throw IPStrNotNullTerm();

	if(inet_pton(AF_INET, ip, &(this->destination.sin_addr)) != 1) throw InvalidIPAddr();
	this->destination.sin_port = htons(port);
}


NPeer::NPeer(const sockaddr_in &addr) noexcept : NPeer()
{
	this->destination.sin_port = addr.sin_port;
	this->destination.sin_addr = addr.sin_addr;
}


NPeer::~NPeer() noexcept
{
	stopNetStream();
	destroyTCP();
}


std::string NPeer::getName() noexcept
{
	if(this->pname[0] == 0)
		return std::string("PeersChatClient");
	else return std::string(this->pname);
}


bool NPeer::setName(std::string name) noexcept
{
	if(name.size() <= 0 || name.size() > MAX_NAME_LEN)
		return false;
	for(unsigned long i = 0; i < name.size(); i++)
	{
		if(std::isalnum(name[i]))
			continue;
		else if((name[i] == '-') && (name[i] == '_'))
			continue;
		else
			return false;
	}

	std::strncpy(this->pname, name.c_str(), MAX_NAME_LEN+1);
	return true;
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
	audio_out_thread.reset(new std::thread(&NPeer::send_audio_over_network_thread, this));
}



void NPeer::stopNetStream() noexcept
{
	run_thread = false;
	std::this_thread::sleep_for(PEERS_CHAT_DESTRUCT_TIMEOUT);
	audio_out_thread.reset();
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


void NPeer::enqueue_in(AudioInPacket *packet)
{
	if(!packet) throw NullPtr();
	else if(packet->packet_len == 0) throw EmptyPack();

	packet->received = steady_clock::now();

	in_queue_lock.lock();
	in_packets.emplace(packet);
	in_queue_lock.unlock();
}


AudioInPacket* NPeer::getAudioInPacket() noexcept
{
	std::unique_ptr<AudioInPacket> packet;
	in_queue_lock.lock();

	// Loop until NULL or valid packet
	do
	{
		packet.reset();
		if(!in_packets.empty())
		{
			// Get a Packet -- If too many packets skip every other
			if(in_packets.size() > IN_PACKET_BUFFER_TOO_LARGE)
				in_packets.pop();
			packet.reset(in_packets.top().get());

			// Packets must idle for a certain amount of time to allow UDP to catch up
			if((steady_clock::now() - packet->received) > PACKET_DELAY)
			{
				const_cast<std::unique_ptr<AudioInPacket>&>(in_packets.top()).release();
				in_packets.pop();
			}
			else
				packet.release();
		}
	} while (packet.get() && (packet->packet_id < in_packet_id));

	in_queue_lock.unlock();
	if(packet.get()) in_packet_id = packet->packet_id;
	return packet.release();
}


bool NPeer::operator==(const sockaddr_in &addr) noexcept
{
	return (destination.sin_port        == addr.sin_port) &&
	       (destination.sin_addr.s_addr == addr.sin_addr.s_addr);
}


bool NPeer::create_udp_socket() noexcept
{
	if (udp > 0) close(udp);
	udp = -1;

	// Create Socket
	if((udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("NPeer::create_udp_socket()");
		udp = -1;
		return false;
	}

	// Bind UDP
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(PORT);
	if(bind(udp, (sockaddr*) &addr, sizeof(addr)) < 0)
	{
		perror("NPeer::create_udp_socket() bind(): ");
		return false;
	}

	return true;
}


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
		#ifdef NET_DEBUG
		if(sent != packet->packet_len)
		{
			std::cerr << "NPeer Audio Out Thread WARNING: Bytes Sent( " << sent << ") != Bytes Intended(" << packet->packet_len << ")\n";
		}
		#endif
	}
}


bool NPeer::createTCP()
{
	if((this->tcp = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return false;

	if(connect(this->tcp, (sockaddr*) &destination, sizeof(this->destination)) < 0)
	{
		destroyTCP();
		return false;
	}

	// Set Timeout
	int64_t timeout = (std::chrono::duration_cast<milliseconds>(SOCKET_TIMEOUT)).count();
	if(setsockopt(this->tcp, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
	{
		destroyTCP();
		return false;
	}
	if(setsockopt(this->tcp, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
	{
		destroyTCP();
		return false;
	}

	return true;
}


void NPeer::destroyTCP()
{
	if(this->tcp < 0) return;
	close(this->tcp);
	this->tcp = -1;
}


// PeersChatNetwork Definitions ----------------------------------------------------------
// Static Helpers
inline static ssize_t send_timeout(int sockfd, const void *buf, size_t len, int flags)
{
	return send(sockfd, buf, len, flags);
}


inline static ssize_t recv_timeout(int sockfd, void *buf, size_t len, int flags)
{
	return recv(sockfd, buf, len, flags);
}


// Constructors
PeersChatNetwork::PeersChatNetwork()
{
	peers.reserve(MAX_PEERS);
}


PeersChatNetwork::~PeersChatNetwork()
{
	this->stop();
}


// Operators
NPeer* PeersChatNetwork::operator[](const sockaddr_in &addr) noexcept
{
	std::lock_guard<std::mutex> lock(this->peers_lock);
	for(int i = 0; i < this->size; ++i)
		if(peers[i].get() && (*peers[i].get() == addr))
			return peers[i].get();
	return NULL;
}


NPeer* PeersChatNetwork::operator[](const int &x) noexcept
{
	std::lock_guard<std::mutex> lock(this->peers_lock);
	if(x >= this->size)
		return NULL;
	return peers[x].get();
}


NPeer* PeersChatNetwork::operator[](const std::string &x) noexcept
{
	// Return a pointer to an NPeer IFF there is exactly one NPeer with that name
	NPeer* ret = NULL;
	std::lock_guard<std::mutex> lock(this->peers_lock);
	for(int i = 0; i < this->size; ++i)
		if(ret && (peers[i]->getName() == x))
			return NULL;
		else if(!ret && (peers[i]->getName() == x))
			ret = peers[i].get();

	return ret;
}


std::string PeersChatNetwork::getMyName() noexcept
{
	if(myName[0] == 0)
		return "PeersChatClient";
	else return this->myName;
}


bool PeersChatNetwork::setMyName(const std::string &name) noexcept
{
	if(name.size() <= 0 || name.size() > MAX_NAME_LEN)
		return false;

	for(unsigned long i = 0; i < name.size(); i++)
	{
		if(std::isalnum(name[i]))
			continue;
		else if((name[i] == '-') && (name[i] == '_'))
			continue;
		else
			return false;
	}

	std::strncpy(this->myName, name.c_str(), MAX_NAME_LEN);
	return true;

}


// Public Functions
bool PeersChatNetwork::join(const sockaddr_in &addr) noexcept
{
	// Make sure everything is stopped first
	if(running) return false;
	this->stop();
	if(this->peers.size() != 0) return false;

	// Create NPeer
	addPeer(addr);
	NPeer *peer = (*this)[addr];

	// Create TCP Connection to Peer
	if(!NPeerAttorney::createTCP(peer))
	{
		return false;
		this->stop();
	}

	// Get TCP socket
	int tcp = NPeerAttorney::getTCP(peer);

	// Request to CONNECT
	connect(tcp);

	// Get Response
	if(!getResponse(tcp))
		return false;

	// Request Peers
	std::vector<sockaddr_in> peer_addr;
	if(!requestPeers(tcp, peer_addr))
		return false;

	// Add Peers
	for(sockaddr_in &addr : peer_addr)
		addPeer(addr);

	// Close Out TCP Connection
	NPeerAttorney::destroyTCP(peer);


	// Start or fail sucessfully
	if(this->start())
	{
		this->getNames();
		return true;
	}
	else
	{
		this->stop();
		return false;
	}
}


void PeersChatNetwork::getNames() noexcept
{
	for(int i = 0; i < this->size; ++i)
	{
		NPeerAttorney::createTCP(this->peers[i].get());
		this->peers[i]->setName(this->getName(NPeerAttorney::getTCP(this->peers[i].get())));
		NPeerAttorney::destroyTCP(this->peers[i].get());
	}
}


bool PeersChatNetwork::host() noexcept
{
	this->stop();
	return this->start();
}


void PeersChatNetwork::disconnect() noexcept
{
	{
		std::lock_guard<std::mutex> lock(this->peers_lock);

		// Open TCP To All Peers
		std::vector<int> peer_fd;
		for(int i = 0; i < this->getNumberPeers(); ++i)
		{
			NPeer *peer = this->peers[i].get();
			NPeerAttorney::createTCP(peer);
			peer_fd.push_back(NPeerAttorney::getTCP(peer));
		}

		// Tell Them you are disconnecting
		for(const int &fd : peer_fd)
			disconnect(fd);
	}

	// End it All
	this->stop();
}


bool PeersChatNetwork::start() noexcept
{
	// Create TCP Server Socket
	if(tcp_listen > 0)
		close(tcp_listen);

	// Sock Create
	if((tcp_listen = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
	{
		perror("PeersChatNetwork::start() socket(): ");
		stop();
		return false;
	}

	// Set Timeout
	int64_t timeout = (std::chrono::duration_cast<milliseconds>(SOCKET_TIMEOUT)).count();
	if(setsockopt(tcp_listen, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
	{
		perror("PeersChatNetwork::start() Failed to set send timeout");
		stop();
		return false;
	}
	if(setsockopt(tcp_listen, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
	{
		perror("PeersChatNetwork::start() Failed to set recv timeout");
		stop();
		return false;
	}

	// Sock Bind
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(PORT);
	if(bind(tcp_listen, (sockaddr*) &addr, sizeof(addr)) < 0)
	{
		perror("PeersChatNetwork::start() bind(): ");
		stop();
		return false;
	}

	// Sock Listen
	if(listen(tcp_listen, MAX_PEERS + 1) < 0)
	{
		perror("PeersChatNetwork::start() listen(): ");
		stop();
		return false;
	}


	// Run Background threads
	running = true;
	listen_thread.reset(new std::thread(&PeersChatNetwork::listen_on_tcp_thread, this));
	recv_thread.reset(new std::thread(&PeersChatNetwork::receive_audio_thread, this));
	for(std::unique_ptr<NPeer> &ptr : this->peers)
		ptr->startNetStream();
	return true;
}


void PeersChatNetwork::stop() noexcept
{
	std::lock_guard<std::mutex> lock(peers_lock);
	running = false;

	this->peers.clear();

	if(recv_thread->joinable() || listen_thread->joinable())
		std::this_thread::sleep_for(PEERS_CHAT_DESTRUCT_TIMEOUT);
	recv_thread.reset();
	listen_thread.reset();

	if(tcp_listen > 0) close(tcp_listen);
	tcp_listen = -1;
}


bool PeersChatNetwork::propose(sockaddr_in &subject, int sock) noexcept
{
	// Tag Type of Request
	uint8_t buffer[7];
	buffer[0] = PROPOSE;

	// Attach IP Address of subject
	union {
		uint32_t addr;
		uint8_t  byte[4];
	} ip;
	ip.addr = subject.sin_addr.s_addr;
	buffer[1] = ip.byte[0];
	buffer[2] = ip.byte[1];
	buffer[3] = ip.byte[2];
	buffer[4] = ip.byte[3];

	// Attach Port Num of subject
	union {
		uint16_t num;
		uint8_t  byte[2];
	} port;
	port.num = subject.sin_port;
	buffer[5] = port.byte[0];
	buffer[6] = port.byte[1];

	// Send Proposal
	if(7 != send_timeout(sock, buffer, 7, MSG_NOSIGNAL))
		return false;
	else return true;
}


bool PeersChatNetwork::respond(bool decision, int sock) noexcept
{
	uint8_t buff;
	if(decision) buff = ACCEPT;
	else buff = DENY;

	if(1 != send_timeout(sock, &buff, 1, 0))
		return false;
	else return true;
}


bool PeersChatNetwork::getResponse(int sock) noexcept
{
	uint8_t buff;
	if(1 != recv_timeout(sock, &buff, 1, MSG_WAITALL))
		return false;
	if (buff == ACCEPT)
		return true;
	else return false;
}


bool PeersChatNetwork::addPeer(const sockaddr_in &addr) noexcept
{
	NPeer *peer = new NPeer(addr);
	if(running) peer->startNetStream();

	std::lock_guard<std::mutex> lock(this->peers_lock);
	if(this->size >= MAX_PEERS) return false;
	this->peers.emplace_back(peer);
	this->size++;
	return true;
}


void PeersChatNetwork::removePeer(sockaddr_in &addr) noexcept
{
	// Search For That Peer
	unsigned long loc;
	for(loc = 0; (loc < peers.size()) && !(*peers[loc] == addr); loc++)
		;

	// They don't exist
	if(loc >= peers.size()) return;

	// They do exist now move them to the back
	{
		std::lock_guard<std::mutex> lock(this->peers_lock);
		this->size--;
		if(peers.size() > 1)
			(peers[loc]).swap(peers[peers.size()-1]);
	}

	// Eliminate Them
	this->peers.pop_back();
}


bool PeersChatNetwork::requestPeers(int sock, std::vector<sockaddr_in> &peers_addr) noexcept
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
	auto min = [](const ssize_t &a, const ssize_t &b) { return (a<b)?a:b; };
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
			ip.byte[0]   = buffer[pos + 0];
			ip.byte[1]   = buffer[pos + 1];
			ip.byte[2]   = buffer[pos + 2];
			ip.byte[3]   = buffer[pos + 3];
			port.byte[0] = buffer[pos + 4];
			port.byte[1] = buffer[pos + 5];
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = ip.num;
			addr.sin_port = port.num;
			peers_addr.push_back(addr);

			#ifdef NET_DEBUG
			char str[INET_ADDRSTRLEN+1] = {0};
			inet_ntop(AF_INET, &addr.sin_addr, str, INET_ADDRSTRLEN);
			std::cout << "Received Peer: " << str << ":" << ntohs(addr.sin_port) << std::endl;
			#endif
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
	uint32_t content_length = this->size * 6;
	word.num = htonl(content_length);
	buffer[1] = word.byte[0];
	buffer[2] = word.byte[1];
	buffer[3] = word.byte[2];
	buffer[4] = word.byte[3];

	// Populate With Peers
	uint32_t pos = 5;
	{
		std::lock_guard<std::mutex> lock(peers_lock);
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
	}
	__attribute__((unused)) ssize_t size = send_timeout(sock, buffer, content_length + 5, MSG_NOSIGNAL);
	#ifdef NET_DEBUG
	if(size != pos || pos != (content_length + 5))
		std::cerr << "PeersChatNetwork::sendPeers() ERROR: Sent Incorrect amount of data" << std::endl;
	#endif
}


void PeersChatNetwork::connect(int sock)
{
	uint8_t buff[3];
	union { uint16_t num; uint8_t byte[2]; }port;
	port.num = htons(PORT);
	buff[0] = CONNECT;
	buff[1] = port.byte[0];
	buff[2] = port.byte[1];
	send_timeout(sock, &buff, 3, MSG_NOSIGNAL);
}


void PeersChatNetwork::disconnect(int sock)
{
	uint8_t buff[3];
	buff[0] = DISCONNECT;
	buff[1] = (uint8_t) ((PORT >> 8) & 0xFF);
	buff[2] = (uint8_t) (PORT & 0xFF);
	send_timeout(sock, &buff, 3, MSG_NOSIGNAL);
}


std::string PeersChatNetwork::getName(int sock) noexcept
{
	// Request Name
	uint8_t buffer[300];
	buffer[0] = REQN;
	buffer[1] = (uint8_t) ((PORT >> 8) & 0xFF);
	buffer[2] = (uint8_t) (PORT & 0xFF);
	if(3 != send_timeout(sock, buffer, 3, MSG_NOSIGNAL))
	{
		#ifdef NET_DEBUG
		std::cerr << "PeersChatNetwork::getName send_timeout(4): Failed" << std::endl;
		#endif
		return "";
	}

	// Receive Data Back and Perform Error Checks
	ssize_t r = recv_timeout(sock, buffer, 300, MSG_WAITALL);
	if(r < 2)
	{
		#ifdef NET_DEBUG
		std::cerr << "PeersChatNetwork::getName recv_timeout(4) Failed" << st::endl;
		#endif
		return "";
	}

	if(buffer[0] != SENDN)
	{
		#ifdef NET_DEBUG
		std::cerr << "PeersChatNetwork::getName Failed: Received wrong type of response" << std::endl;
		#endif
		return "";
	}

	// Parse Name
	std::string name = "";
	for(unsigned i = 0; i < buffer[2]; ++i)
		name.push_back((char)buffer[3 + i]);

	return name;
}


void PeersChatNetwork::receive_audio_thread()
{
	uint8_t buffer[BUFFER_SIZE];

	ssize_t r = 0;
	sockaddr_in addr;
	while(running)
	{
		memset(&addr, 0, sizeof(addr));

		// Receive Packet
		r = recvfrom(NPeerAttorney::getUDP(), buffer, BUFFER_SIZE,
		             MSG_WAITALL, (sockaddr*) &addr, (socklen_t*) sizeof(addr));
		if(r < 0) continue;
		if(buffer[0] != SENDV) continue;

		// Sort
		NPeer *peer = (*this)[addr];
		if(!peer)
		{
			#ifdef NET_DEBUG
			char str[INET_ADDRSTRLEN+1] = {0};
			inet_ntop(AF_INET, &addr.sin_addr, str, INET_ADDRSTRLEN);
			std::cout << "Packet from unknown source " << str << ":" << ntohs(addr.sin_port) << std::endl;
			#endif
			continue;
		}

		// Get Empty Packet
		std::unique_ptr<AudioInPacket> pack(peer->getEmptyInPacket());

		// Set Packet ID, Packet Length, and Copy Data
		pack->packet_id  = (buffer[1] << 24) | (buffer[2] << 16) | (buffer[3] << 8) | (buffer[4]);
		pack->packet_len = (buffer[5] << 24) | (buffer[6] << 16) | (buffer[7] << 8) | (buffer[8]);
		std::memcpy(pack->packet.get(), buffer + 9, pack->packet_len);

		// Queue
		peer->enqueue_in(pack.release());
	}
}


void PeersChatNetwork::listen_on_tcp_thread()
{
	int peer = -1;
	uint8_t buffer[BUFFER_SIZE];
	sockaddr_in addr;
	while(running)
	{
		if(peer > 0) close(peer);
		peer = -1;

		// Accept connection
		if((peer = accept(tcp_listen, (sockaddr*) &addr, (socklen_t*) sizeof(addr))) < 0)
		{
			perror("PeersChatNetwork::listen_on_tcp_thread() accept: ");
			continue;
		}

		// Get Req Type
		if(1 != recv_timeout(peer, buffer, 1, MSG_WAITALL)) continue;

		// Handle Req Type
		if(buffer[0] == CONNECT) //--------------------------------------------------
		{
			if(2 != recv_timeout(peer, &addr.sin_port, 2, MSG_WAITALL)) continue;

			#ifdef NET_DEBUG
			inet_ntop(AF_INET, &addr.sin_addr, (char*)buffer, 50);
			printf("CONNECT Request from %s:%" PRIu16 "\n", buffer, ntohs(addr.sin_port));
			connectFulfill(peer, addr);
			#endif
		}
		else if(buffer[0] == PROPOSE) //---------------------------------------------
		{
			#ifdef NET_DEBUG
			if(inet_ntop(AF_INET, &addr.sin_addr, (char*) buffer, 50) == NULL) continue;

			#endif
			recv_timeout(peer, &addr.sin_addr.s_addr, 4, MSG_WAITALL);
			recv_timeout(peer, &addr.sin_port, 2, MSG_WAITALL);
			#ifdef NET_DEBUG
			printf("PROPOSE Request from %s to add ", buffer);
			if(inet_ntop(AF_INET, &addr.sin_addr, (char*) buffer, 50) == NULL) continue;
			printf("%s:%" PRIu16 "\n", buffer, ntohs(addr.sin_port));
			#endif
			proposeFulfill(peer, addr);
		}
		else if(buffer[0] == DISCONNECT) //------------------------------------------
		{
			if(2 != recv_timeout(peer, &addr.sin_port, 2, MSG_WAITALL)) continue;
			removePeer(addr);
		}
		else if(buffer[0] == REQN) //------------------------------------------------
		{
			// Get Data and Find Peer
			if(2 != recv_timeout(peer, &addr.sin_port, 2, MSG_WAITALL)) continue;
			NPeer *peer_ptr = (*this)[addr];

			#ifdef NET_DEBUG
			inet_ntop(AF_INET, &addr.sin_addr, std::memset(buffer, 0, INET_ADDRSTRLEN+1) , INET_ADDRSTRLEN);
			printf("REQN request from %s:%" PRIu16 "\n", buffer, ntohs(addr.sin_port));
			#endif

			// Requester is not affiliated with you
			if(!peer_ptr)
			{
				std::cerr << "Failed REQN Request: Peer Not Recognized" << std::endl;
				continue;
			}

			// Send them your name
			std::string name = this->getMyName();
			buffer[0] = SENDN;
			buffer[1] = (uint8_t) name.length();
			for(int i = 0; i < buffer[1]; ++i)
				buffer[2 + i] = name[i];
			send_timeout(peer, buffer, 2 + buffer[2], MSG_NOSIGNAL);
		} // ------------------------------------------------------------------------

		// End Request
		close(peer);
	}
}


bool PeersChatNetwork::connectFulfill(int new_member, sockaddr_in addr)
{
	bool connect = accept_direct_join && (this->size < MAX_PEERS);
	if(!connect) return false;

	// Open TCP to peers
	std::vector<int> peer_fd;
	for(std::unique_ptr<NPeer> &p : this->peers)
	{
		connect &= NPeerAttorney::createTCP(p.get());
		if(!connect) break;
		peer_fd.push_back(NPeerAttorney::getTCP(p.get()));

	}

	// Close Them If Failed
	if(!connect)
	{
		#ifdef NET_DEBUG
		std::cerr << "PeersChatNetwork::connectFulfill failed to open TCP to Peers" << std::endl;
		#endif
		uint8_t buff = CLOSE;
		for(const int &fd : peer_fd)
		{
			if(fd > 0)
				send_timeout(fd, &buff, 1, MSG_NOSIGNAL);
		}
		for(auto &p : this->peers)
			NPeerAttorney::destroyTCP(p.get());
		return false;
	}

	// Propose letting new guy join to everyone else
	for(const int &fd : peer_fd)
		connect &= propose(addr, fd);

	// Get Their Responses
	for(const int &fd : peer_fd)
		connect &= getResponse(fd);

	#ifdef NET_DEBUG
	std::cout << "CONNECT request result: " << (connect?"Approved":"Denied") << std::endl;
	#endif

	// Let Everyone Know The Result
	for(const int &fd : peer_fd)
		respond(connect, fd);
	respond(connect, new_member);

	// Receive REQP Request
	uint8_t tag;
	if(1 != recv_timeout(new_member, &tag, 1, MSG_WAITALL))
		connect = false;
	if(tag != REQP) connect = false;
	#ifdef NET_DEBUG
	std::cout << "REQP Received" << std::endl;
	#endif

	// SendP
	sendPeers(new_member);

	// Add Peer Yourself
	addPeer(addr);

	// Get His Name
	NPeer *peer_ptr = (*this)[addr];
	if(peer_ptr)
	{
		NPeerAttorney::createTCP(peer_ptr);
		peer_ptr->setName(this->getName(NPeerAttorney::getTCP(peer_ptr)));
		NPeerAttorney::destroyTCP(peer_ptr);
	}

	#ifdef NET_DEBUG
	std::cout << "Peer Successfully Added" << std::endl;
	#endif
	return true;
}


/*
 * Param @peer is a peer you are already connected to asking you if his friend
 * at @addr can join the call.
 */
bool PeersChatNetwork::proposeFulfill(int peer, sockaddr_in addr)
{
	// Let him know if his friend can join
	bool decision = accept_indirect_join && (this->size < MAX_PEERS);
	respond(decision, peer);

	#ifdef NET_DEBUG
	std::cout << (decision?"Accepted":"Declined") << " proposal" << std::endl;
	#endif

	// Get his response on whether the friend is joining
	uint8_t tag = DENY;
	recv_timeout(peer, &tag, 1, MSG_WAITALL);

	#ifdef NET_DEBUG
	std::cout << "Peer " << ((tag==ACCEPT)?"ACCEPTED":"DENIED") << " his friend joining" << std::endl;
	#endif

	if(tag == ACCEPT && decision)
	{
		addPeer(addr);
		NPeer *peer_ptr = (*this)[addr];
		if(peer_ptr)
		{
			NPeerAttorney::createTCP(peer_ptr);
			peer_ptr->setName(this->getName(NPeerAttorney::getTCP(peer_ptr)));
			NPeerAttorney::destroyTCP(peer_ptr);
		}
		return true;
	}
	else return false;
}


// This Library was written in its entirety by Omar Ahmadyar (oahmadyar@gmail.com)
