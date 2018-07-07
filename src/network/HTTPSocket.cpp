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
	std::cout << buffer->RawBuf << std::endl;
	Read();
}

void HTTPSocket::OnWrite(RIOBuffer* buffer, int transferred)
{

}

void HTTPSocket::OnConnected()
{
	std::cout << "connected http socket" << std::endl;
	Read();
}

void HTTPSocket::OnClose()
{
	std::cout << "close http socket" << std::endl;
}