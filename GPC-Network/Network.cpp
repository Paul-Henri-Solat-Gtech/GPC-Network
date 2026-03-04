#include "Network.h"

bool Network::Init(bool _isServer, int _serverPort)
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

void Network::Close()
{
	if (m_pHost != NULL)
	{
		std::cout << "Closing session...\n";
		enet_host_destroy(m_pHost);
	}
}

void Network::ServerLoop()
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
				{
					std::cout << "New Client has joinned !" << std::endl;
					m_clients.push_back(event.peer);
					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					std::cout << "Client disconnected !" << std::endl;
					/*auto it = std::remove(m_clients.begin(),m_clients.end(),event.peer);
					m_clients.erase(it, m_clients.end());*/
					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
				{
					package = reinterpret_cast<Package*>(event.packet->data);
					std::cout << "Recieved Package : " << event.packet->data << std::endl;
					SendMsgToClients("test");
					ShowSyncVars();
					enet_packet_destroy(event.packet);
					break;
				}
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

bool Network::SendMsgToClients(const char* _message)
{
	for (auto client : m_clients)
	{
		ENetPacket* packet = enet_packet_create(_message, strlen(_message) + 1, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(client, 0, packet);
		enet_host_flush(m_pHost);
	}

	enet_host_flush(m_pHost);
	return true;
}

void Network::ShowSyncVars()
{
	SendMsgToClients("---SyncVars---");

	auto& registry = SyncRegistry::Instance().Get();

	if (registry.empty())
	{
		std::cout << "[ShowSyncVars] registry empty!\n";
	}

	for (auto& syncVar : registry)
	{
		std::string name = syncVar.first;
		const SyncEntry& entry = syncVar.second;

		// PRINT SERVER-SIDE (debug)
		std::cout << "[ShowSyncVars] " << name << " type=" << static_cast<int>(entry.type) << " size=" << entry.size << " ptr=" << entry.data << "\n";

		// envoie vers clients
		SendMsgToClients(name.c_str());
		SendMsgToClients("=");
		switch (entry.type)
		{
		case SyncType::INT:
			SendMsgToClients(std::to_string(*static_cast<int*>(entry.data)).c_str());
			break;
		case SyncType::FLOAT:
			SendMsgToClients(std::to_string(*static_cast<float*>(entry.data)).c_str());
			break;
		case SyncType::BOOL:
			SendMsgToClients((*static_cast<bool*>(entry.data)) ? "true" : "false");
			break;
		case SyncType::STRING:
			SendMsgToClients(static_cast<std::string*>(entry.data)->c_str());
			break;
		default:
			SendMsgToClients("Unknown");
		}
	}

	SendMsgToClients("--------------");
}

bool Network::ConnectingTo(const char* _addressIP, int _addressPort)
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

				std::cout << "Server sent : " << event.packet->data << std::endl;

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

void Network::DisconnectFromServer()
{
	if (m_pServerConnection)
	{
		std::cout << "Disconnecting from " << m_pServerConnection->address.host << " | " << m_pServerConnection->address.port << "...\n";
		enet_peer_disconnect(m_pServerConnection, 0);
		Close();
	}
}

bool Network::SendMsgToServer()
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

bool Network::SendMsgToServer(const char* _message)
{
	ENetPacket* packet = enet_packet_create(_message, strlen(_message) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(m_pServerConnection, 0, packet);
	enet_host_flush(m_pHost);
	return true;
}

void Network::CommandManager(std::string command)
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

void Network::SyncVarsToClients(const std::string& name, const void* data, size_t size)
{
	Package pkg{};
	strncpy_s(pkg.name, name.c_str(), sizeof(pkg.name));
	pkg.dataSize = static_cast<int>(size);
	memcpy(pkg.data, data, size);

	for (auto client : m_clients)
	{
		ENetPacket* packet = enet_packet_create(&pkg, sizeof(pkg), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(client, 0, packet);
	}

	enet_host_flush(m_pHost);
}
