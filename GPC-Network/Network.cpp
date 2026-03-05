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

					SendMsgToClients("I will give you my sync vars.");
					SyncVarsToClients();

					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					std::cout << "Client disconnected !" << std::endl;
					auto it = std::remove(m_clients.begin(),m_clients.end(),event.peer);
					m_clients.erase(it, m_clients.end());
					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
				{
					if (event.packet->dataLength == sizeof(Package))
					{
						Package* package = reinterpret_cast<Package*>(event.packet->data);

						ReceiveSyncVar(package);
						PrintSyncVar();
					}
					else
					{
						std::cout << "Client msg : " << event.packet->data << std::endl;
					}
					enet_packet_destroy(event.packet);
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
		case SyncType::STRING:
			SendMsgToClients(static_cast<std::string*>(entry.data)->c_str());
			break;
		case SyncType::BOOL:
			SendMsgToClients((*static_cast<bool*>(entry.data)) ? "true" : "false");
			break;
		case SyncType::INT:
			SendMsgToClients(std::to_string(*static_cast<int*>(entry.data)).c_str());
			break;
		case SyncType::FLOAT:
			SendMsgToClients(std::to_string(*static_cast<float*>(entry.data)).c_str());
			break;
		default:
			SendMsgToClients("Unknown");
		}
	}

	SendMsgToClients("--------------");
}

void Network::PrintSyncVar()
{
	std::cout << "---SyncVars---\n";

	auto& registry = SyncRegistry::Instance().Get();

	if (registry.empty())
	{
		std::cout << "[ShowSyncVars] registry empty!\n";
	}

	for (auto& syncVar : registry)
	{
		std::string name = syncVar.first;
		const SyncEntry& entry = syncVar.second;

		std::cout << name.c_str() << ": ";
		switch (entry.type)
		{
		case SyncType::INT:
		{
			std::cout << "INT=";
			std::cout << std::to_string(*static_cast<int*>(entry.data)).c_str();
			break;
		}
		case SyncType::FLOAT:
		{
			std::cout << "FLOAT=";
			std::cout << std::to_string(*static_cast<float*>(entry.data)).c_str();
			break;
		}
		case SyncType::BOOL:
		{
			std::cout << "BOOL=";
			bool value = *static_cast<bool*>(entry.data);
			std::cout << (value ? "true" : "false");
			break;
		}
		case SyncType::STRING:
		{
			std::cout << "STRING=";
			std::cout << static_cast<std::string*>(entry.data)->c_str();
			break;
		}
		default:
			std::cout << "ERROR ";
		}

		std::cout << " size=" << entry.size << std::endl;
	}

	std::cout << "------------" << std::endl;
}

void Network::SyncVarsToClients()
{
	if (!m_clients.empty())
	{
		auto& registry = SyncRegistry::Instance().Get();

		for (auto& [name, entry] : registry)
		{
			Package pkg{};

			memset(pkg.name, 0, sizeof(pkg.name));
			size_t nameLen = name.size();
			if (nameLen > sizeof(pkg.name) - 1)nameLen = sizeof(pkg.name) - 1;
			memcpy(pkg.name, name.c_str(), nameLen);

			if (entry.type == SyncType::STRING)
			{
				std::string* s = static_cast<std::string*>(entry.data);
				size_t dataLen = s->size();

				if (dataLen > sizeof(pkg.data))dataLen = sizeof(pkg.data);

				pkg.dataSize = (int)dataLen;
				pkg.type = entry.type;
				memcpy(pkg.data, s->c_str(), dataLen);
			}
			else
			{
				size_t sendSize = entry.size;

				if (sendSize > sizeof(pkg.data))sendSize = sizeof(pkg.data);

				pkg.dataSize = (int)sendSize;
				pkg.type = entry.type;
				memcpy(pkg.data, entry.data, sendSize);
			}

			for (auto client : m_clients)
			{
				ENetPacket* packet = enet_packet_create(&pkg, sizeof(pkg), ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(client, 0, packet);
				std::cout << "Client" << client->connectID << " (syncing vars...)" << std::endl;
			}
		}

		enet_host_flush(m_pHost);
	}
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
			{
				std::cout << "Succesfully connected to Enet Server !" << std::endl;
				connected = true;
				SendMsgToServer("bruh");
				break;
			}
			case ENET_EVENT_TYPE_RECEIVE:
			{
				if (event.packet->dataLength == sizeof(Package))
				{
					Package* package = reinterpret_cast<Package*>(event.packet->data);

					ReceiveSyncVar(package);
					PrintSyncVar();
				}
				else
				{
					std::cout << "Server msg : " << event.packet->data << std::endl;
				}
				enet_packet_destroy(event.packet);

				break;
			}
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
	if (command[0] == '/')
	{
		if (command == "/close")
		{
			Close();
		}
		if (command == "/disconnect")
		{
			DisconnectFromServer();
		}
		if (command == "/sendToServ")
		{
			SendSyncVar();
		}
	}
}

void Network::ReceiveSyncVar(Package* package)
{
	std::cout << "Receiving syncVars..." << std::endl;

	size_t nameLen = strnlen(package->name, sizeof(package->name));
	std::string pkgName(package->name, nameLen);

	auto& registry = SyncRegistry::Instance().Get();

	// Supprimer ancien entry proprement
	auto it = registry.find(pkgName);
	if (it != registry.end())
	{
		SyncEntry& old = it->second;

		if (old.data && old.ownerAllocated)
		{
			if (old.type == SyncType::STRING)
				delete static_cast<std::string*>(old.data);
			else
				free(old.data);
		}

		registry.erase(it);
	}

	// Nouvelle entry
	SyncEntry entry{};
	entry.size = static_cast<size_t>(package->dataSize);
	entry.type = package->type;
	entry.ownerAllocated = true;

	if (entry.size > sizeof(package->data))
		entry.size = sizeof(package->data);

	if (package->type == SyncType::STRING)
	{
		std::string* s = new std::string(
			package->data,
			package->data + entry.size
		);

		entry.data = s;
	}
	else
	{
		void* mem = malloc(entry.size ? entry.size : 1);
		memcpy(mem, package->data, entry.size);
		entry.data = mem;
	}

	SyncRegistry::Instance().Register(pkgName, entry);

	std::cout << "Received pkg: " << pkgName
		<< " size=" << entry.size << std::endl;
}

void Network::SendSyncVar()
{
	auto& registry = SyncRegistry::Instance().Get();

	for (auto& [name, entry] : registry)
	{
		Package pkg{};

		memset(pkg.name, 0, sizeof(pkg.name));
		size_t nameLen = name.size();
		if (nameLen > sizeof(pkg.name) - 1)nameLen = sizeof(pkg.name) - 1;
		memcpy(pkg.name, name.c_str(), nameLen);

		if (entry.type == SyncType::STRING)
		{
			std::string* s = static_cast<std::string*>(entry.data);
			size_t dataLen = s->size();

			if (dataLen > sizeof(pkg.data))dataLen = sizeof(pkg.data);

			pkg.dataSize = (int)dataLen;
			pkg.type = entry.type;
			memcpy(pkg.data, s->c_str(), dataLen);
		}
		else
		{
			size_t sendSize = entry.size;

			if (sendSize > sizeof(pkg.data))sendSize = sizeof(pkg.data);

			pkg.dataSize = (int)sendSize;
			pkg.type = entry.type;
			memcpy(pkg.data, entry.data, sendSize);
		}

		ENetPacket* packet = enet_packet_create(&pkg, sizeof(pkg), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(m_pServerConnection, 0, packet);
		std::cout << "Server" << m_pServerConnection->connectID << " (syncing vars...)" << std::endl;
	}

	enet_host_flush(m_pHost);
}