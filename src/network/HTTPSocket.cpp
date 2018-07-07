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
	
	auto* newBuffer = g_RIOBufferPool->AllocBuffer();
	newBuffer->Length = BUFFER_SIZE;
	Read(newBuffer);
}

void HTTPSocket::OnWrite(RIOBuffer* buffer, int transferred)
{

}

void HTTPSocket::OnConnected()
{
	std::cout << "connected http socket" << std::endl;

	auto* buffer = g_RIOBufferPool->AllocBuffer();
	buffer->Length = BUFFER_SIZE;
	Read(buffer);
}

void HTTPSocket::OnClose()
{
	std::cout << "close http socket" << std::endl;
}