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
};

PeersChatNetwork *Network = NULL;
APeer *Audio = NULL;
PC_GuiHandler *GUI = NULL;

int main(const int argc, char *argv[])
{	
	// Create PeersChat Object
	std::unique_ptr<PeersChat> pchat(new PeersChat);

	// Make Network accessible globaly
	Network = &(pchat->network);
	Audio = &(pchat->audio);
	GUI = &(pchat->GUI);

	pchat->GUI.runGui(argc,argv);

	return EXIT_SUCCESS;
}
