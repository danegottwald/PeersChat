#include "PC_Server.h"

void netErrorCheck(const std::string& message, int error, bool critical);

char PC_Server::recvBuffer[]{};

/* PC_Server Default Constructor
 *
 */
PC_Server::PC_Server() {
	netResultServer = WSAStartup(MAKEWORD(2,2), &WinSockData);
	if (netResultServer != NO_ERROR) std::cerr << "WSAStartUp failed";
	UDPServerSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (UDPServerSocket == INVALID_SOCKET) std::cerr << "Socket failed";
	int optval = 1;
	int iResult = ::setsockopt(UDPServerSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&optval), sizeof optval);
	if (iResult == SOCKET_ERROR) {
		printf("setsockopt failed with error: %d\n", WSAGetLastError());
	}
	recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	recvAddr.sin_port = htons(8181);
	recvAddr.sin_family = AF_INET;
	netResultServer = bind(UDPServerSocket, (SOCKADDR*)&recvAddr, sizeof(recvAddr));
	if (netResultServer != 0) std::cerr << "Bind failed" << WSAGetLastError();

}

PC_Server::PC_Server(const char *IP, u_short port) {
	netResultServer = WSAStartup(MAKEWORD(2,2), &WinSockData);
	if (netResultServer != NO_ERROR) std::cerr << "WSAStartUp failed";
	UDPServerSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (UDPServerSocket == INVALID_SOCKET) std::cerr << "Socket failed";
	recvAddr.sin_addr.s_addr = inet_addr(IP); // 192.168.1.124    127.0.0.1
	recvAddr.sin_port = htons(port); // 8181       8001
	recvAddr.sin_family = AF_INET;
	netResultServer = bind(UDPServerSocket, (SOCKADDR*)&recvAddr, sizeof(recvAddr));
	if (netResultServer != 0) std::cerr << "Bind failed";

	/*int optval = 1;
	int iResult = ::setsockopt(UDPServerSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&optval), sizeof optval);
	if (iResult == SOCKET_ERROR) {
		printf("setsockopt failed with error: %d\n", WSAGetLastError());
	}*/
}

PC_Server::~PC_Server() {
	netResultServer = closesocket(UDPServerSocket);
	if (netResultServer == SOCKET_ERROR) std::cerr << "Close socket error";
	WSACleanup();
}

char* PC_Server::checkForPacket() {
	// Non blocking will allow this function to pass if no packet is received
	fd_set readSet;
	struct timeval timeout;
	while (true) {
		FD_ZERO(&readSet);
		FD_SET(UDPServerSocket, &readSet);
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		netResultServer = select(0, &readSet, nullptr, nullptr, &timeout);
		if (netResultServer > 0) {
			//netResultServer = recv(UDPServerSocket, recvBuffer, UDPClientLen, 0);
			netResultServer = recvfrom(UDPServerSocket, recvBuffer, MAX_NETPACKET_SIZE, 0, (SOCKADDR*)&recvAddr, &recvAddrSize);
			//std::cout << recvBuffer << std::endl;
			if (netResultServer == SOCKET_ERROR) {
				netResultServer = WSAGetLastError();
				if (netResultServer == WSAEWOULDBLOCK) {
					continue;
				}
				std::cerr << "Socket error " << WSAGetLastError();
				exit(EXIT_FAILURE);
			}
			if (netResultServer == 0) {
				std::cout << "Connection closed" << std::endl;
			}
			continue;
			return recvBuffer;
			//*recvBuffer += netResultServer;
			//UDPClientLen -= netResultServer;
		}
		else {
			// IF TIMEOUT, RUN ...
			//std::cout << "Timeout" << std::endl;
		}
	}
	return nullptr;
}

void PC_Server::packetCheckThreaded() {
	networkThread = std::thread(&PC_Server::checkForPacket, this);
}

void PC_Server::printLocalAddress() {
	char ac[80];
	if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
		exit(10);
	}
	std::cout << "Host name: " << ac << std::endl;
	struct hostent *phe = gethostbyname(ac);
	for (int i = 0; phe->h_addr_list[i] != 0; i++) {
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
		std::cout << "Address: " << inet_ntoa(addr) << std::endl;
	}
}
