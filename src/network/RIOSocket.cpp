#include "stdafx.h"
#include "RIOSocket.h"
#include "RIOServer.h"
#include "RIOBuffer.h"

RIOSocket::RIOSocket(SOCKET rawSock, const SOCKADDR_IN& addr)
	: RawSocket(rawSock)
	, Addr(addr)
	, RequestQueue(RIO_INVALID_RQ)
	, Server(nullptr)
{

}

void RIOSocket::Initialize(RIO_RQ requestQueue, RIOServer* server)
{
	RequestQueue = requestQueue;
	Server = server;

	OnConnected();
}

void RIOSocket::OnIOCallBack(int status, RIOBuffer* buffer, int transferred)
{
	if (status)
	{
		auto error = WSAGetLastError();
		std::cout << "dequeue completion status error status : " << status << "error : " << error << std::endl;
		Close();
	}

	if (!transferred)
		Close();

	if (buffer->Type == RequestType::RIO_READ)
		OnRead(buffer);


	DecreaseRef();
}

void RIOSocket::Read(RIOBuffer* buffer)
{

}

void RIOSocket::Write(RIOBuffer* buffer)
{

}

void RIOSocket::Close()
{
	auto before = RawSocket.exchange(INVALID_SOCKET);
	if (before != INVALID_SOCKET)
	{
		closesocket(before);
		OnClose();
		Server->DeleteSocket(this);
	}
}

SOCKET RIOSocket::GetRawSocket() const
{
	return RawSocket.load();
}