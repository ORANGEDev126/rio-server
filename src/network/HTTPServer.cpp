#include "stdafx.h"
#include "HTTPServer.h"
#include "HTTPSocket.h"

HTTPServer::HTTPServer(int threadCount)
	: RIOServer(threadCount)
{
}

RIOSocket* HTTPServer::CreateSocket(SOCKET rawSocket, const SOCKADDR_IN& addr)
{
	return new HTTPSocket(rawSocket, addr);
}

int HTTPServer::GetPort()
{
	return 80;
}