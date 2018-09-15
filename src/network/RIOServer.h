#pragma once

#include "RIOThreadContainer.h"
#include "RIOSocketContainer.h"

namespace network
{
class RIOServer
{
public:
	RIOServer(int threadCount);
	~RIOServer();

	void Run();
	void Stop();
	void DeleteSocket(RIOSocket* sock);

private:
	void AcceptLoop();
	virtual RIOSocket* CreateSocket(SOCKET rawSock, const SOCKADDR_IN& addr) = 0;
	virtual int GetPort() = 0;

	SOCKET listenSocket;
	bool stop;
	std::unique_ptr<std::thread> acceptThread;
	RIOThreadContainer threadContainer;
	RIOSocketContainer socketContainer;
};
}