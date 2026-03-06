/*
GPC-Network.cpp :
To be clean, even if the p1 is the host the server part and client part are separated.
*/

#include <enet/enet.h>
#include <iostream>

#include "Network.h"

int main()
{
	// var sync with clients
	SyncVar(std::string, "Name") playerName("Player1");
	SyncVar(int, "HP") playerHp = 10;
	playerHp = 55;
	SyncVar(float, "PosX") playerPosX = 5.56f;
	SyncVar(bool, "IsDead") playerIsDead = false;


	// Enet INIT
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}
	std::cout << "Enet Program launched.\n";

	// Server INIT
	Network testNetwork;

	std::string localIp = testNetwork.GetLocalIP();
	std::cout << "NETWORK IP = " << localIp << std::endl;
	if (testNetwork.Init(true, 54321))
	{
		testNetwork.PrintSyncVar();
		testNetwork.ServerLoop();
		//testNetwork.Close();
	}

	// Program END
	atexit(enet_deinitialize);
	return 0;
}
