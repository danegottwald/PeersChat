#include <iostream>
#include <string>
#include "getopt.h"

int main(const int argc, char *argv[])
{	
	std::string user, address; // no flag for ip address
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
					user = optarg;
					break;
				default:
					return EXIT_FAILURE;
			}
		}
		address = argv[optind];
		std::cout << address << std::endl;
	}
	else
	{
		std::cout << "GUI mode" << std::endl;
	}
	return EXIT_SUCCESS;
}

