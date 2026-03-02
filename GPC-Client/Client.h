#ifndef FOUNDRY_CLIENT__H_
#define FOUNDRY_CLIENT__H_

#include <enet/enet.h>
#include <iostream>

struct Package 
{
};

struct PackagePlayer : Package
{
	int posX, posY, posZ;
	std::string name;
};

class Client
{
public:
	Client() = default;

	bool Init();
	void Close();

	bool ConnectingTo(const char* _addressIP, int _addressPort);
	void DisconnectFromServer();
	bool SendMsgToServer(const char* _message);
	bool SendDataToServer(Package _packageToSend);

protected:
	ENetHost* m_pClient;
	ENetPeer* m_pServerConnection;

private:

};

#endif