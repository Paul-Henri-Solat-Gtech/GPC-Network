#ifndef FOUNDRY_NETWORK__H_
#define FOUNDRY_NETWORK__H_

#include <enet/enet.h>
#include <iostream>

class Network
{
public:
	Network() = default;
	
	bool Init();
	void Close();

	void ServerLoop();

protected:
	ENetAddress m_address;
	ENetHost* m_server;

private:

};

#endif

