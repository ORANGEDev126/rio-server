#pragma once

#include "RIOServer.h"

namespace network
{
class HTTPServer : public RIOServer
{
public:
	HTTPServer(int threadCount);

private:
	virtual RIOSocket* CreateSocket(SOCKET rawSocket, const SOCKADDR_IN& addr) override;
	virtual int GetPort() override;
};
}