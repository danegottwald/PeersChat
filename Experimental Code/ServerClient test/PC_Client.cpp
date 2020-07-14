#include "PC_Client.h"

int PC_Client::netResultClient = 0;
SOCKET PC_Client::sendSocket{};
sockaddr_in PC_Client::UDPServer{};
char PC_Client::sendOut[MAX_NETPACKET_SIZE]{};

PC_Client::PC_Client() {
	WSAStartup(MAKEWORD(2, 2), &WinSockData);
	netResultClient = WSAStartup(MAKEWORD(2, 2), &WinSockData);
	if (netResultClient != 0) std::cerr << "WSAStartUp Failed";
}

PC_Client::PC_Client(const char *serverIP, const u_short port) {
	netResultClient = WSAStartup(MAKEWORD(2, 2), &WinSockData);
	if (netResultClient != 0) std::cerr << "WSAStartUp Failed";
	UDPServer.sin_family = AF_INET;
	UDPServer.sin_addr.s_addr = inet_addr(serverIP);
	UDPServer.sin_port = htons(port);
	sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

PC_Client::~PC_Client() {
	netResultClient = closesocket(sendSocket);
	if (netResultClient == SOCKET_ERROR) std::cerr << "Close socket error";
	WSACleanup();
}

void PC_Client::sendPacket() {
	netResultClient = sendto(sendSocket, sendOut, MAX_NETPACKET_SIZE, MSG_DONTROUTE, (SOCKADDR*) &UDPServer, sizeof(UDPServer));
	//netResultServer = sendto(sendSocket, "Hello!", MAX_NETPACKET_SIZE, MSG_DONTROUTE, (SOCKADDR*) &UDPServer, sizeof(UDPServer));
}

void PC_Client::setServerAddress(const char *serverIP, u_short port) {
	UDPServer.sin_family = AF_INET;
	UDPServer.sin_addr.s_addr = inet_addr(serverIP);
	UDPServer.sin_port = htons(port);
	sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sendSocket == INVALID_SOCKET) std::cerr << "Socket create error";
}
