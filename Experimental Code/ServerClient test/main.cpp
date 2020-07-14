#include <iostream>

//#include "PC_Network.h"
#include "PC_Audio.h"
#include "PC_Server.h"

int main() {

	PC_Server server;
	server.printLocalAddress();

	PC_AudioHandler handler;

	server.packetCheckThreaded();
	handler.startVoiceStream();
	//handler.setOutputVolume(0);

	int choose;
	std::cout << "1. Host\n2. Connect\n";
	std::cin >> choose;
	if (choose == 1) {

	}
	else if (choose == 2) {

	}


	/*int val;
	do {
		std::cin >> val;
		switch (val) {
			case 1:
				handler.stopVoiceStream();
				break;
			case 2:
				handler.setOutputVolume(0);
				break;
			case 3:
				handler.setOutputVolume(.5f);
				break;
			case 4:
				handler.startVoiceStream();
				break;
		}
	} while (val != 0);*/
	std::cin.get();

	return 0;
}
