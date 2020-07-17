#include "PC_Gui.hpp"

int main(int argc, char *argv[])
{
	PC_GuiHandler* gh = new PC_GuiHandler();
	gh->RunGui(argc, argv);
}
