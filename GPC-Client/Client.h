#ifndef FOUNDRY_CLIENT__H_
#define FOUNDRY_CLIENT__H_

#include <enet/enet.h>
#include <iostream>
#include <unordered_map>

struct Package
{
	char name[20];
	int dataSize;
	char data[255];
};

template <typename T, const char* Name>
struct Syncvar
{
public:
	Syncvar(T data) : m_Data(data)
	{
		//map[Name] = this;
		// strlengt(Name) find how to add it to the map
	}

	~Syncvar()
	{
		//map.erase(Name);
	}

	void OnChange()
	{
		//logique reseau
	}

	void operator=(T other)
	{
		m_Data = other;
		OnChange();
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

	bool Init();
	void Close();

	bool ConnectingTo(const char* _addressIP, int _addressPort);
	void DisconnectFromServer();
	bool SendMsgToServer(const char* _message);
	bool SendDataToServer(Package& _packageToSend);

	void SyncVarsToServer();

protected:
	ENetHost* m_pClient;
	ENetPeer* m_pServerConnection;

	std::unordered_map<std::string, void*> m_mapSyncVar;
private:

};

#endif