#ifndef FOUNDRY_CLIENT__H_
#define FOUNDRY_CLIENT__H_

#include <enet/enet.h>
#include <iostream>

class Client
{
public:
	Client() = default;

	bool Init();
	void Close();

	void ConnectingToEnetServer();

protected:
	ENetHost* m_client;

private:

};

#endif