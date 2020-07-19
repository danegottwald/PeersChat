#include <string>
#include <unistd.h>
#include "PC_Network.hpp"
#include "PC_Audio.h"
#include "PC_GUI.hpp"

class PeersChat
{
	public:
		NPeer network;
		PC_AudioHandler audio;
		PC_GuiHandler GUI;
		bool useGUI = true;
		std::string user; 
		std::string address; // no flag for ip address
};

int main(const int argc, char *argv[])
{	
	PeersChat peer;
	if(argc > 1)
	{
		peer.useGUI = false;
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

		while(true)
		{
			// Grab user input

		}
	}
	else
	{
		std::cout << "GUI mode" << std::endl;
		peer.GUI.runGui(argc,argv);
	}

	return EXIT_SUCCESS;
}
