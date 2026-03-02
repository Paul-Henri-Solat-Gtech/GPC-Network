// GPC-Client.cpp

/*
Both the host and the p2 will use this class to be created and join the server.
*/

#include <enet/enet.h>
#include <iostream>

#include "Client.h"

int main()
{
    // Enet INIT
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }

    // Network INIT
    Client testClient;
    if (testClient.Init()) 
    {
        testClient.ConnectingToEnetServer();
    }

    // Program END
    atexit(enet_deinitialize);
    return 0;
}