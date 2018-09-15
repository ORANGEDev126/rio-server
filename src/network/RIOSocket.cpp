#include "stdafx.h"
#include "RIOSocket.h"
#include "RIOserver.h"
#include "RIOBuffer.h"
#include "RIOBufferPool.h"
#include "RIOStreamBuffer.h"

namespace network
{
RIOSocket::RIOSocket(SOCKET rawSock, const SOCKADDR_IN& client)
	: rawSocket(rawSock)
	, addr(client)
	, requestQueue(RIO_INVALID_RQ)
	, server(nullptr)
{

}

void RIOSocket::Initialize(RIO_RQ queue, RIOServer* owner)
{
	requestQueue = queue;
	server = owner;

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
		if (buffer->type == REQUEST_TYPE::RIO_READ)
			OnReadCallBack(buffer, transferred);
		else if (buffer->type == REQUEST_TYPE::RIO_WRITE)
			OnWriteCallBack(buffer, transferred);
	}

	DecRef();
}

void RIOSocket::OnReadCallBack(RIOBuffer* buffer, int transferred)
{
	buffer->size += transferred;

	for (;;)
	{
		RIOStreamBuffer streamBuf(readBuf);
		std::istream packet(&streamBuf);

		int packetLength = PacketSize(packet);
		int currLength = static_cast<int>(streamBuf.showmanyc());

		packetLength = packetLength == 0 ? currLength : packetLength;

		if (currLength >= packetLength)
		{
			int left = currLength - packetLength;
			buffer->size -= left;

			OnRead(packet);

			std::copy(buffer->rawBuf + buffer->size, buffer->rawBuf + buffer->size + left, readBuf.front()->rawBuf);
			readBuf.front()->size = left;

			while (readBuf.size() != 1)
			{
				auto* buf = readBuf.back();
				readBuf.pop_back();
				RIOBufferPool::GetInstance()->Free(buf);
			}

			if (left == 0)
				break;
		}
		else
		{
			break;
		}
	}

	Read();
}

void RIOSocket::OnWriteCallBack(RIOBuffer* buffer, int transferred)
{
	OnWrite(buffer, transferred);
	RIOBufferPool::GetInstance()->Free(buffer);
}

void RIOSocket::Read()
{
	if (readBuf.empty() || readBuf.back()->size == BUFFER_SIZE)
		readBuf.push_back(RIOBufferPool::GetInstance()->Alloc());

	auto* buf = readBuf.back();
	buf->Offset = buf->size;
	buf->Length = BUFFER_SIZE - buf->size;
	buf->type = REQUEST_TYPE::RIO_READ;

	IncRef();
	std::lock_guard<std::mutex> lock(requestLock);
	if (!g_RIO.RIOReceive(requestQueue, buf, 1, NULL, buf))
	{
		auto error = WSAGetLastError();
		PrintConsole(std::string("receive request failed ") + std::to_string(error));
		DecRef();
		Close();
	}
}

bool RIOSocket::Write(RIOBuffer* buffer)
{
	buffer->Length = buffer->size;
	buffer->type = REQUEST_TYPE::RIO_WRITE;

	IncRef();
	std::lock_guard<std::mutex> lock(requestLock);
	if (!g_RIO.RIOSend(requestQueue, buffer, 1, NULL, buffer))
	{
		auto error = WSAGetLastError();
		PrintConsole(std::string("send request failed ") + std::to_string(error));
		RIOBufferPool::GetInstance()->Free(buffer);
		DecRef();
		Close();
		return false;
	}

	return true;
}

void RIOSocket::Close()
{
	auto before = rawSocket.exchange(INVALID_SOCKET);
	if (before != INVALID_SOCKET)
	{
		closesocket(before);
		OnClose();
		server->DeleteSocket(this);
	}
}

SOCKET RIOSocket::GetRawSocket() const
{
	return rawSocket.load();
}
}