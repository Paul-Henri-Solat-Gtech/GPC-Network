#ifndef FOUNDRY_NETWORK__H_
#define FOUNDRY_NETWORK__H_

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


class Network
{
public:
	Network() = default;
	
	bool Init(int _serverPort);
	void Close();

	void ServerLoop();

protected:
	ENetAddress m_address;
	ENetHost* m_pServer;

private:

};

#endif

