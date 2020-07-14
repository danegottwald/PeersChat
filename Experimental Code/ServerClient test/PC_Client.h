#ifndef _PC_Client_H
#define _PC_Client_H

#include <iostream>
#include <winsock2.h>

#define MAX_NETPACKET_SIZE 60

class PC_Client {
private:
	WSADATA WinSockData{};
	int iWsaStartup{};
	char buffer[MAX_NETPACKET_SIZE] = "";
	int iBufferLen = strlen(buffer) + 1;
	int iUDPServerLen = sizeof(UDPServer);

protected:
	static char sendOut[MAX_NETPACKET_SIZE];
	static int netResultClient;
	static SOCKET sendSocket;
	static struct sockaddr_in UDPServer;
public:
	PC_Client();
	PC_Client(const char *serverIP, const u_short port);
	~PC_Client();

	static void sendPacket();
	void setServerAddress(const char* serverIP, u_short port);

};

#endif //_PC_Client_H
