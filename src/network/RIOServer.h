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

private:
	void AcceptLoop();
	virtual int GetPort() = 0;

	std::function<std::shared_ptr<RIOSocket>(SOCKET, const SOCKADDR_IN&)> sockAllocator;
	SOCKET listenSocket;
	bool stop;
	std::unique_ptr<std::thread> acceptThread;
	std::shared_ptr<RIOThreadContainer> threadContainer;
	std::shared_ptr<RIOSocketContainer> socketContainer;
};
}