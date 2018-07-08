#include "stdafx.h"
#include "RIOSocket.h"
#include "RIOServer.h"
#include "RIOBuffer.h"
#include "RIOBufferPool.h"

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
	if (status || !transferred)
	{
		Close();
	}
	else
	{
		if (buffer->Type == RequestType::RIO_READ)
			OnRead(buffer, transferred);
		else if (buffer->Type == RequestType::RIO_WRITE)
			OnWrite(buffer, transferred);
	}

	g_RIOBufferPool->FreeBuffer(buffer);
	DecRef();
}

void RIOSocket::Read()
{
	auto* buffer = g_RIOBufferPool->AllocBuffer();
	buffer->Type = RequestType::RIO_READ;
	buffer->Length = BUFFER_SIZE;

	IncRef();

	if (!g_RIO.RIOReceive(RequestQueue, static_cast<RIO_BUF*>(buffer), 1, 0, buffer))
	{
		auto error = WSAGetLastError();
		PrintConsole(std::string("RIO receive error ") + std::to_string(error));
		DecRef();
		Close();
		return;
	}
}

void RIOSocket::Write(RIOBuffer* buffer)
{
	buffer->Type = RequestType::RIO_WRITE;
	IncRef();

	if (!g_RIO.RIOSend(RequestQueue, static_cast<RIO_BUF*>(buffer), 1, 0, buffer))
	{
		auto error = WSAGetLastError();
		PrintConsole(std::string("RIO send error ") + std::to_string(error));
		DecRef();
		Close();
		return;
	}
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