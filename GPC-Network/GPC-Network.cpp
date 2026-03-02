/*
GPC-Network.cpp :
To be clean, even if the p1 is the host the server part and client part are separated.
*/

#include <enet/enet.h>
#include <iostream>

#include "Network.h"

int main()
{
	// Enet INIT
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}
	std::cout << "Enet Program launched.\n";

	// Network INIT
	Network testNetwork;

	if (testNetwork.Init(54321))
	{
		testNetwork.ServerLoop();
		testNetwork.Close();
	}

	// Program END
	atexit(enet_deinitialize);
	return 0;
}
