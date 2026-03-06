/*
GPC-Client.cpp :
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

    // Client INIT
    Client testClient;
    if (testClient.Init()) 
    {
        testClient.PrintSyncVar();
        if(testClient.ConnectingTo("127.0.0.1", 54321)) 
        {
            //testClient.ClientLoop();
            if (testClient.SendMsgToServer("HI ! Im going to send you my data(client).")) 
            {
                //testClient.SendSyncVar();
                testClient.SendMsgToServerA();
            }
        }
    }

    // Program END
    atexit(enet_deinitialize);
    return 0;
}