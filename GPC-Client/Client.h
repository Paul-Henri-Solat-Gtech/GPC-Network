#ifndef FOUNDRY_NETWORK__H_
#define FOUNDRY_NETWORK__H_

#include <enet/enet.h>
#include <iostream>
#include <unordered_map>

class Client;

struct Package
{
	char name[20];
	int dataSize;
	char data[255];
};

struct SyncRegistry
{
	static SyncRegistry& Instance()
	{
		static SyncRegistry instance;
		return instance;
	}

	void Register(const std::string& name, void* ptr)
	{
		m_registry[name] = ptr;
	}

	void Unregister(const std::string& name)
	{
		m_registry.erase(name);
	}

	std::unordered_map<std::string, void*>& Get()
	{
		return m_registry;
	}

private:
	std::unordered_map<std::string, void*> m_registry;
};

template <typename T, const char* Name>
struct Syncvar
{
public:
	Syncvar(T data) : m_Data(data)
	{
		SyncRegistry::Instance().Register(Name, this);
	}

	~Syncvar()
	{
		SyncRegistry::Instance().Unregister(Name);
	}

	void OnChange()
	{
		//logique reseau
		Client::Instance().SyncVarsToClients(Name, &m_Data, sizeof(T));
	}

	void operator=(T other)
	{
		m_Data = other;
		OnChange();
	}

	int Size() const
	{
		return sizeof(T);
	}

	const void* Data() const
	{
		return &m_Data;
	}

private:
	T m_Data;
	const char* m_name = Name;
};

#define SyncVar(type, name) static const char __name__[] = name; Syncvar<type, __name__>

class Client
{
public:
	Client() = default;

	bool Init(bool _isServer = false, int _serverPort = 0);
	void Close();

	// Server Host
	void ServerLoop();
	bool SendMsgToClients(const char* _message);

	// Simple Client
	bool ConnectingTo(const char* _addressIP, int _addressPort);
	void DisconnectFromServer();

	bool SendMsgToServer();
	bool SendMsgToServer(const char* _message);
	void CommandManager(std::string command);

	void SyncVarsToClients(const std::string& name, const void* data, size_t size);

	static Client& Instance()
	{
		static Client instance;
		return instance;
	}

protected:
	ENetAddress m_address;
	ENetHost* m_pHost = nullptr; // can be host(server) or client
	ENetPeer* m_pServerConnection = nullptr;
	std::vector<ENetPeer*> m_clients;

	std::unordered_map<std::string, void*> m_mapSyncVar;

private:
	bool m_isServer = false;
};

#endif

