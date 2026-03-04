#include "Client.h"

bool Client::Init(bool _isServer, int _serverPort)
{
	m_isServer = _isServer;

	if (m_isServer)
	{
		//server
		m_address.host = ENET_HOST_ANY;
		m_address.port = _serverPort;
		m_pHost = enet_host_create(&m_address, 32, 2, 0, 0);

		std::cout << "Server Initialized !\n";
	}
	else
	{
		m_pHost = enet_host_create(NULL, 1, 2, 0, 0);

		std::cout << "Client Initialized !\n";
	}


	if (m_pHost == NULL)
	{
		fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
		exit(EXIT_FAILURE);
		return false;
	}

	return true;
}

void Client::Close()
{
	if (m_pHost != NULL)
	{
		std::cout << "Closing session...\n";
		enet_host_destroy(m_pHost);
	}
}

void Client::ServerLoop()
{
	if (m_isServer)
	{
		ENetEvent event;
		Package* package = nullptr;
		while (true)
		{
			while (enet_host_service(m_pHost, &event, 1000) > 0)
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
					SendMsgToClients("test");
					enet_packet_destroy(event.packet);
					break;
				default:
					break;
				}
			}
		}
	}
	else
	{
		std::cout << "Host not initialized has server [!] " << std::endl;
	}
}

bool Client::SendMsgToClients(const char* _message)
{
	for (auto client : m_clients)
	{
		ENetPacket* packet = enet_packet_create(_message, strlen(_message) + 1, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(client, 0, packet);
		enet_host_flush(m_pHost);
	}

	return true;
}

bool Client::ConnectingTo(const char* _addressIP, int _addressPort)
{
	ENetAddress address;
	enet_address_set_host(&address, _addressIP);
	address.port = _addressPort;

	m_pServerConnection = enet_host_connect(m_pHost, &address, 2, 0);
	if (!m_pServerConnection)
	{
		std::cerr << "Connection failed to server." << std::endl;
		enet_host_destroy(m_pHost);
		return false;
	}

	// Wait for connection
	ENetEvent event;
	Package* package = nullptr;
	bool connected = false;
	int maxTries = 10;
	for (int i = 0; i < maxTries && !connected; ++i)
	{
		while (enet_host_service(m_pHost, &event, 1000) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				std::cout << "Succesfully connected to Enet Server !" << std::endl;
				connected = true;
				SendMsgToServer("bruh");
				//SendMsgToServer();
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				//SendMsgToServer();
				//m_mapSyncVar = event.packet->data; // copy mcpy map
				package = reinterpret_cast<Package*>(event.packet->data);

				//memcpy(m_mapSyncVar[package->name], package->data, package->dataSize);

				std::cout << "Recieved Package : " << event.packet->data << std::endl;

				enet_packet_destroy(event.packet);

				//loop
				SendMsgToServer();

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

bool Client::SendMsgToServer()
{
	std::cout << "Enter msg: ";
	std::string msg;
	std::cin >> msg;

	CommandManager(msg);

	ENetPacket* packet = enet_packet_create(msg.c_str(), strlen(msg.c_str()) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(m_pServerConnection, 0, packet);
	enet_host_flush(m_pHost);
	return true;
}

bool Client::SendMsgToServer(const char* _message)
{
	ENetPacket* packet = enet_packet_create(_message, strlen(_message) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(m_pServerConnection, 0, packet);
	enet_host_flush(m_pHost);
	return true;
}

void Client::CommandManager(std::string command)
{
	if (command[0] == '/') //Slash = command
	{
		if (command == "/close")
		{
			Close();
		}
		if (command == "/disconnect")
		{
			DisconnectFromServer();
		}
	}
}

void Client::SyncVarsToClients(const std::string& name, const void* data, size_t size)
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
