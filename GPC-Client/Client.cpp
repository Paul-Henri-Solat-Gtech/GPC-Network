#include "Client.h"

bool Client::Init() 
{
    m_pClient = enet_host_create(NULL,1,2,0,0);

    if (m_pClient == NULL)
    {
        fprintf(stderr, "An error occurred while trying to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }

    std::cout << "Client Created !\n";

	return true;
}

void Client::Close()
{
    if (m_pClient != NULL)
    {
        std::cout << "Closing session...\n";
        enet_host_destroy(m_pClient);
    }
}

void Client::ConnectingTo(const char* _addressIP, int _addressPort)
{
    ENetAddress address;
    enet_address_set_host(&address, _addressIP);
    address.port = _addressPort;

    ENetPeer* peer = enet_host_connect(m_pClient, &address, 2, 0);
    if (!peer)
    {
        std::cerr << "Connection failed to server." << std::endl;
        enet_host_destroy(m_pClient);
        return;
    }

    // Wait for connection
    ENetEvent event;
    bool connected = false;
    for (int i = 0; i < 10 && !connected; ++i)
    {
        while (enet_host_service(m_pClient, &event, 1000) > 0)
        {
            if (event.type == ENET_EVENT_TYPE_CONNECT)
            {
                std::cout << "Succesfully connected to Enet Server !" << std::endl;
                connected = true;
                const char* message = "Hello server !";
                ENetPacket* packet = enet_packet_create(message, strlen(message) + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(peer, 0, packet);
            }
        }
        if (!connected) std::cout << "Connection Try " << i + 1 << "/10..." << std::endl;
    }
    if (!connected)
    {
        std::cerr << "Failed connection (timeout)." << std::endl;
    }
    enet_peer_disconnect(peer, 0);
}
