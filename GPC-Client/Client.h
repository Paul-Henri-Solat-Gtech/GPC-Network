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

	void ConnectingTo(const char* _addressIP, int _addressPort);

protected:
	ENetHost* m_pClient;

private:

};

#endif