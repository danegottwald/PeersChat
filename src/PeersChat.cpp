#include <string>
#include <unistd.h>
#include <PC_Network.hpp>
#include <PC_Audio.hpp>
#include <PC_Gui.hpp>

class PeersChat
{
	public:
		NPeer peer;
		PeersChatNetwork network;
		APeer audio;
		PC_GuiHandler GUI;
		bool useGUI = true;
};

int main(const int argc, char *argv[])
{	
	PeersChat pchat;
	if(argc > 1)
	{
		pchat.useGUI = false;
		int choice;
		while((choice = getopt(argc,argv,"hj:u:")) != -1)
		{
			switch(choice)
			{
				case 'h': // host a room
					pchat.network.host();
					break;
				case 'j': // join a room, need an address
					pchat.network.join(optarg);
					break;
				case 'u': // enter a username
					pchat.peer.setName(optarg);
					break;
				default:
					return EXIT_FAILURE;
			}
		}

		while(true)
		{
			std::string input;
			// Grab user input for other functions (volume, kick, leave)
			getline(cin,input);

			if(input == "-q")
			{
				pchat.network.disconnect();
				break;
			}

			if(input == "-k") // -k (user)
			{
				// Kick someone
			}

			if(input == "-v") // -v (+/- num)
			{
				//set volume
				pchat.audio.setOutputVolume((float) input); // still needs string parsing
			}

			if(input == "-m") // -m (user)
			{
				//mute someone
				pchat.audio.setMuteMic(true);
			}

		}
	}
	else
	{
		// std::cout << "GUI mode" << std::endl;
		peer.GUI.runGui(argc,argv);
	}

	return EXIT_SUCCESS;
}
