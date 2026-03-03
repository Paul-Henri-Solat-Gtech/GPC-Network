#include "Client.h"

bool Client::Init()
{
	m_pClient = enet_host_create(NULL, 1, 2, 0, 0);

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

bool Client::ConnectingTo(const char* _addressIP, int _addressPort)
{
	ENetAddress address;
	enet_address_set_host(&address, _addressIP);
	address.port = _addressPort;

	m_pServerConnection = enet_host_connect(m_pClient, &address, 2, 0);
	if (!m_pServerConnection)
	{
		std::cerr << "Connection failed to server." << std::endl;
		enet_host_destroy(m_pClient);
		return false;
	}

	// Wait for connection
	ENetEvent event;
	bool connected = false;
	int maxTries = 10;
	for (int i = 0; i < maxTries && !connected; ++i)
	{
		while (enet_host_service(m_pClient, &event, 1000) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				std::cout << "Succesfully connected to Enet Server !" << std::endl;
				connected = true;
				SendMsgToServer("bruh");
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				//m_mapSyncVar = event.packet->data; // copy mcpy map
				break;
			default:
				break;
			}
		}
		if (!connected) std::cout << "Connection Try " << i + 1 << "/" << maxTries << "...\n";
	}
	if (!connected)
	{
		std::cerr << "Failed connection (timeout)." << std::endl;
		return false;
	}

	return true;
}

void Client::DisconnectFromServer()
{
	if (m_pServerConnection)
	{
		std::cout << "Disconnecting from " << m_pServerConnection->address.host << " | " << m_pServerConnection->address.port << "...\n";
		enet_peer_disconnect(m_pServerConnection, 0);
	}
}

bool Client::SendMsgToServer(const char* _message)
{
	ENetPacket* packet = enet_packet_create(_message, strlen(_message) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(m_pServerConnection, 0, packet);
	enet_host_flush(m_pClient);
	return true;
}

bool Client::SendDataToServer(Package& _packageToSend)
{
	ENetPacket* packet = enet_packet_create(&_packageToSend, sizeof(_packageToSend) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(m_pServerConnection, 0, packet);
	enet_host_flush(m_pClient);
	return true;
}

void Client::SyncVarsToServer()
{
}