#ifndef _PC_NetworkTest_H
#define _PC_NetworkTest_H

/*
 * TODO: Modify this section for OS compatibility
 * This library is initially written for GNU/Linux systems
 * Includes will have to be moved in/out of guards for cross-platform support
 */

// Universal Includes
#include <iostream>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#define MAX_NETPACKET_SIZE 60

class PC_Server {
private:
	WSADATA WinSockData{};
	SOCKET UDPServerSocket;
	struct sockaddr_in recvAddr;
	int recvAddrSize = sizeof(recvAddr);
	int UDPClientLen;
	int netResultServer;

protected:
	static char recvBuffer[MAX_NETPACKET_SIZE];

public:
	PC_Server();
	PC_Server(const char *IP, u_short port);
	~PC_Server();

	std::thread networkThread;

	char* checkForPacket();
	void packetCheckThreaded();

	void printLocalAddress();
};


#endif//_PC_NetworkTest_H