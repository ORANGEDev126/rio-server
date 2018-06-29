#pragma once

#include "RIOThreadContainer.h"
#include "RIOSocketContainer.h"

class RIOServer
{
public:
	RIOServer(int threadCount);
	void Run();
	
private:
	void AcceptLoop();

	RIOThreadContainer ThreadContainer;
	RIOSocketContainer SocketContainer;
};