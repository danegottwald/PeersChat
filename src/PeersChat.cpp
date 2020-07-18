#include <string>
#include "getopt.h"
#include "PC_Network.hpp"
#include "PC_Audio.h"
#include "PC_GUI.hpp"

class PeersChat
{
	private:
		NPeer network;
		PC_AudioHandler audio;
		PC_GuiHandler GUI;
	public:
		bool useGUI = true;
		std::string user; 
		std::string address; // no flag for ip address

};

int main(const int argc, char *argv[])
{	
	PeersChat peer;
	if(argc > 1)
	{
		int choice;
		while((choice = getopt(argc,argv,"hu:")) != -1)
		{
			switch(choice)
			{
				case 'h': // host a room
					break;
				case 'u': // enter a username
					peer.user = optarg;
					break;
				default:
					return EXIT_FAILURE;
			}
		}
		peer.address = argv[optind];
		std::cout << peer.address << std::endl;
	}
	else
	{
		std::cout << "GUI mode" << std::endl;
	}
	return EXIT_SUCCESS;
}

