/*
GPC-Network.cpp :
To be clean, even if the p1 is the host the server part and client part are separated.
*/

#include <enet/enet.h>
#include <iostream>

#include "Network.h"

int main()
{
	SyncVar(int, "testVal") testVal = 10;
	testVal = 55;

	// Enet INIT
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}
	std::cout << "Enet Program launched.\n";

	// Server INIT
	Network testNetwork;

	if (testNetwork.Init(true, 54321))
	{
		testNetwork.ServerLoop();
		testNetwork.Close();
	}

	// Program END
	atexit(enet_deinitialize);
	return 0;
}
