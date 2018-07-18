#include "stdafx.h"
#include "HTTPServer.h"
#include "HTTPSocket.h"

namespace network
{
HTTPServer::HTTPServer(int threadCount)
	: RIOServer(threadCount)
{
}

RIOSocket* HTTPServer::CreateSocket(SOCKET rawSocket, const SOCKADDR_IN& addr)
{
	PrintConsole("new socket");
	return new HTTPSocket(rawSocket, addr);
}

int HTTPServer::GetPort()
{
	return 80;
}
}