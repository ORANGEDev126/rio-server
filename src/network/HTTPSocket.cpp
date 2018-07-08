#include "stdafx.h"
#include "HTTPSocket.h"
#include "RIOBufferPool.h"
#include "RIOBuffer.h"

HTTPSocket::HTTPSocket(SOCKET rawSocket, const SOCKADDR_IN& addr)
	: RIOSocket(rawSocket, addr)
{
}

void HTTPSocket::OnRead(RIOBuffer* buffer, int transferred)
{
	PrintConsole(buffer->RawBuf);
	Read();
}

void HTTPSocket::OnWrite(RIOBuffer* buffer, int transferred)
{

}

void HTTPSocket::OnConnected()
{
	PrintConsole("connected http socket");
	Read();
}

void HTTPSocket::OnClose()
{
	PrintConsole("close http socket");
}