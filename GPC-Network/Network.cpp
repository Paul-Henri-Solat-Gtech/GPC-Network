#include "Network.h"

bool Network::Init()
{
	m_address.host = ENET_HOST_ANY;
	m_address.port = 54321;
    m_server = enet_host_create(&m_address,32,2,0,0);

    if (m_server == NULL)
    {
        fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
        return false;
    }

	return true;
}

void Network::Close()
{
    if (m_server != NULL)
    {
        std::cout << "Closing server...\n";
        enet_host_destroy(m_server);
    }
}

void Network::ServerLoop()
{
    ENetEvent event;
    while (true)
    {
        while (enet_host_service(m_server, &event, 1000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                std::cout << "New Client has joinned !" << std::endl;
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                std::cout << "Recieved Package : " << event.packet->data << std::endl;
                enet_packet_destroy(event.packet);
                break;
            default:
                break;
            }
        }
    }
}
