#ifndef FOUNDRY_NETWORK__H_
#define FOUNDRY_NETWORK__H_

#include <enet/enet.h>
#include <iostream>
#include <unordered_map>
#include <type_traits>
#include <string>
#include <thread>

class Network;

enum class SyncType
{
	INT,
	FLOAT,
	STRING,
	BOOL,

	DEFAULT
};

struct Package
{
	char name[20];
	int dataSize;
	char data[255];
	SyncType type;
};

struct SyncEntry
{
	void* data = nullptr;
	SyncType type = SyncType::DEFAULT;
	size_t size = 0;
	bool ownerAllocated = false;
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
	Syncvar(const T& data) : m_Data(data)
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
		//Network::Instance().SyncVarsToClients(Name,&m_Data,sizeof(T));
		Network::Instance().SyncVarsToClients();
	}

	Syncvar& operator=(const T& other)
	{
		if (m_Data != other)
		{
			m_Data = other;
			OnChange();
		}

		return *this;
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

//#define SyncVar(type, name) static const char __name__[] = name; Syncvar<type, __name__>
#define CONCAT_IMPL(x,y) x##y
#define CONCAT(x,y) CONCAT_IMPL(x,y)

#define SyncVar(type, name) \
static const char CONCAT(__name__, __LINE__)[] = name; \
Syncvar<type, CONCAT(__name__, __LINE__)>

class Network
{
public:
	Network() = default;
	
	bool Init(bool _isServer = false, int _serverPort = 0);
	void Close();

	// Server Host
	void ServerLoop();
	bool SendMsgToClients(const char* _message);
	void PrintSyncVar();
	void SyncVarsToClients();

	// Simple Client
	bool ConnectingTo(const char* _addressIP, int _addressPort);
	void ClientLoop();
	void DisconnectFromServer();

	bool SendMsgToServerA();
	bool SendMsgToServer(const char* _message);
	void CommandManager(std::string command);
	void ReceiveSyncVar(Package* package);
	void SendSyncVar();

	static Network& Instance()
	{
		static Network instance;
		return instance;
	}

protected:
	ENetAddress m_address;
	ENetHost* m_pHost = nullptr; // can be host(server) or client
	ENetPeer* m_pServerConnection = nullptr;
	std::vector<ENetPeer*> m_clients;

private:
	bool m_isServer = false;
	bool m_isRunning = false;
	bool m_isConnected = false;
	std::thread m_networkThread;
	std::thread m_inputThread;
};

#endif

