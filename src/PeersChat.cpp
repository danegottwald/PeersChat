#include <string>
#include <unistd.h>
#include <PC_Network.hpp>
#include <PC_Audio.hpp>
#include <PC_Gui.hpp>

class PeersChat
{
	public:
		PeersChatNetwork network;
		APeer audio;
		PC_GuiHandler GUI;
		bool useGUI = true;
};

PeersChatNetwork *Network = NULL;
APeer *Audio = NULL;

int main(const int argc, char *argv[])
{	
	// Create PeersChat Object
	std::unique_ptr<PeersChat> pchat(new PeersChat);

	// Make Network accessible globaly
	Network = &(pchat->network);
	Audio = &(pchat->audio);

	if(argc > 1)
	{
		pchat->useGUI = false;
		int choice;
		while((choice = getopt(argc,argv,"hj:u:")) != -1)
		{
			switch(choice)
			{
				case 'h': // host a room
					pchat->network.host();
					break;
				case 'j': // join a room, need an address
//					pchat->network.join(optarg);
					break;
				case 'u': // enter a username
					pchat->network.setMyName(optarg);
					break;
				default:
					return EXIT_FAILURE;
			}
		}

		while(true)
		{
			// Print Users
			for(int i = 0; i < pchat->network.getNumberPeers(); ++i)
				std::cout << i << ": " << pchat->network[i]->getName() << '\n';

			// Grab user input for other functions (volume, kick, leave)
			std::string input;
			getline(std::cin,input);

			if(input == "q")
			{
				pchat->network.disconnect();
				break;
			}

			if(input == "k") // -k (user)
			{
				// Kick someone
			}

			if(input == "v") // -v (+/- num)
			{
				//set volume
//				pchat->audio.setOutputVolume((float) input); // still needs string parsing
			}

			if(input == "m") // -m (user)
			{
				//mute someone
				pchat->audio.setMuteMic(true);
			}

		}
	}
	else
	{
		// std::cout << "GUI mode" << std::endl;
		pchat->GUI.runGui(argc,argv);
	}

	return EXIT_SUCCESS;
}
