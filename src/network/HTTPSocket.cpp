#include "stdafx.h"
#include "HTTPSocket.h"
#include "RIOBufferPool.h"
#include "RIOBuffer.h"

namespace network
{
HTTPSocket::HTTPSocket(SOCKET rawSocket, const SOCKADDR_IN& addr)
	: RIOSocket(rawSocket, addr)
{
}

void HTTPSocket::OnRead(std::istream& buf)
{
	std::string read(std::istreambuf_iterator<char>(buf), {});
	PrintConsole(read);
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

int HTTPSocket::PacketSize(std::istream& packet)
{
	return 0;
}
}