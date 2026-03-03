#include "Network.h"

bool Network::Init(int _serverPort)
{
	m_address.host = ENET_HOST_ANY;
	m_address.port = _serverPort;
	m_pServer = enet_host_create(&m_address, 32, 2, 0, 0);

	if (m_pServer == NULL)
	{
		fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
		exit(EXIT_FAILURE);
		return false;
	}

	return true;
}

void Network::Close()
{
	if (m_pServer != NULL)
	{
		std::cout << "Closing server...\n";
		enet_host_destroy(m_pServer);
	}
}

void Network::ServerLoop()
{
	ENetEvent event;
	Package* package = nullptr;
	while (true)
	{
		while (enet_host_service(m_pServer, &event, 1000) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				std::cout << "New Client has joinned !" << std::endl;
				m_clients.push_back(event.peer);
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				
				package = reinterpret_cast<Package*>(event.packet->data);
				
				//memcpy(m_mapSyncVar[package->name], package->data, package->dataSize);

				std::cout << "Recieved Package : " << event.packet->data << std::endl;

				enet_packet_destroy(event.packet);
				break;
			default:
				break;
			}
		}
	}
}

void Network::SyncVarsToClients(const std::string& name, const void* data, size_t size)
{
	Package pkg{};

	strncpy_s(pkg.name, name.c_str(), sizeof(pkg.name));
	pkg.dataSize = size;

	memcpy(pkg.data, data, size);

	ENetPacket* packet = enet_packet_create(&pkg, sizeof(pkg), ENET_PACKET_FLAG_RELIABLE);

	for (auto client : m_clients)
	{
		enet_peer_send(client, 0, packet);
	}
}
