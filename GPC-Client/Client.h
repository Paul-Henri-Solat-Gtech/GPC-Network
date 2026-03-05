#ifndef FOUNDRY_NETWORK__H_
#define FOUNDRY_NETWORK__H_

#include <enet/enet.h>
#include <iostream>
#include <unordered_map>
#include <type_traits>
#include <string>

class Client;

struct Package
{
	char name[20];
	int dataSize;
	char data[255];
};

enum class SyncType
{
	INT,
	FLOAT,
	STRING,
	BOOL,

	DEFAULT
};

struct SyncEntry
{
	void* data = nullptr;
	SyncType type = SyncType::DEFAULT;
	size_t size = 0;
};

struct SyncRegistry
{
	static SyncRegistry& Instance()
	{
		static SyncRegistry instance;
		return instance;
	}

	void Register(const std::string& name, const SyncEntry& entry)
	{
		m_registry.insert_or_assign(name, entry);
	}

	void Unregister(const std::string& name)
	{
		m_registry.erase(name);
	}

	std::unordered_map<std::string, SyncEntry>& Get()
	{
		return m_registry;
	}

private:
	std::unordered_map<std::string, SyncEntry> m_registry;
};

template <typename T, const char* Name>
struct Syncvar
{
public:
	Syncvar(T data) : m_Data(data)
	{
		SyncEntry entry;
		entry.data = &m_Data;
		entry.size = sizeof(T);
		entry.type = DeduceType();

		SyncRegistry::Instance().Register(Name, entry);
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

protected:
	static SyncType DeduceType()
	{
		if constexpr (std::is_same_v<T, int>)
			return SyncType::INT;
		else if constexpr (std::is_same_v<T, float>)
			return SyncType::FLOAT;
		else if constexpr (std::is_same_v<T, bool>)
			return SyncType::BOOL;
		else if constexpr (std::is_same_v<T, std::string>)
			return SyncType::STRING;
		return SyncType::DEFAULT;
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
	void ShowSyncVars();

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

	std::unordered_map<std::string, SyncEntry> m_mapSyncVar;
private:
	bool m_isServer = false;
};

#endif

