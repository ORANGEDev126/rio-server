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
{

}

RIOSocket::~RIOSocket()
{
	FreeAllReadBuf();
}

void RIOSocket::Initialize(RIO_RQ queue, const std::shared_ptr<RIOSocketContainer>& socketContainer)
{
	requestQueue = queue;
	container = socketContainer;

	Read();

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
		buffer->size += transferred;

		if (buffer->type == REQUEST_TYPE::RIO_READ)
			OnReadCallBack(buffer);
		else if (buffer->type == REQUEST_TYPE::RIO_WRITE)
			OnWriteCallBack(buffer);
	}
}

void RIOSocket::OnReadCallBack(RIOBuffer* buffer)
{
	for (;;)
	{
		RIOStreamBuffer streamBuf(readBuf);
		std::istream packet(&streamBuf);

		int packetLength = PacketSize(packet);
		int currLength = static_cast<int>(streamBuf.showmanyc());

		packetLength = packetLength == 0 ? currLength : packetLength;

		if (currLength < packetLength)
			break;

		int left = currLength - packetLength;
		buffer->size -= left;

		OnRead(packet);
		
		std::copy(buffer->rawBuf + buffer->size,
				  buffer->rawBuf + buffer->size + left, readBuf.front()->rawBuf);
		buffer->size = left;

		FreeReadBufUntilLast();

		if (left == 0)
			break;
	}

	Read();
}

void RIOSocket::OnWriteCallBack(RIOBuffer* buffer)
{
	RIOBufferPool::GetInstance()->Free(buffer);
}

void RIOSocket::FreeReadBufUntilLast()
{
	if (readBuf.empty() || readBuf.size() == 1)
		return;

	auto* last = readBuf.back();
	readBuf.pop_back();

	for (auto* buf : readBuf)
		RIOBufferPool::GetInstance()->Free(buf);

	readBuf.clear();
	readBuf.push_back(last);
}

void RIOSocket::FreeAllReadBuf()
{
	for (auto* buf : readBuf)
		RIOBufferPool::GetInstance()->Free(buf);

	readBuf.clear();
}

void RIOSocket::Read()
{
	if (readBuf.empty() || readBuf.back()->size == BUFFER_SIZE)
		readBuf.push_back(RIOBufferPool::GetInstance()->Alloc());

	auto* buf = readBuf.back();
	buf->Offset = buf->size;
	buf->Length = BUFFER_SIZE - buf->size;
	buf->type = REQUEST_TYPE::RIO_READ;

	std::lock_guard<std::mutex> lock(requestLock);
	if (!g_RIO.RIOReceive(requestQueue, buf, 1, NULL, buf))
	{
		auto error = WSAGetLastError();
		PrintConsole(std::string("receive request failed ") + std::to_string(error));
		Close();
	}

	selfContainer.push_back(shared_from_this());
}

bool RIOSocket::Write(RIOBuffer* buffer)
{
	buffer->Length = buffer->size;
	buffer->type = REQUEST_TYPE::RIO_WRITE;

	std::lock_guard<std::mutex> lock(requestLock);
	if (!g_RIO.RIOSend(requestQueue, buffer, 1, NULL, buffer))
	{
		auto error = WSAGetLastError();
		PrintConsole(std::string("send request failed ") + std::to_string(error));
		RIOBufferPool::GetInstance()->Free(buffer);
		Close();
		return false;
	}

	selfContainer.push_back(shared_from_this());

	return true;
}

void RIOSocket::Close()
{
	auto before = rawSocket.exchange(INVALID_SOCKET);
	if (before != INVALID_SOCKET)
	{
		if (auto socketContainer = container.lock())
			socketContainer->DeleteSocket(shared_from_this());

		closesocket(before);
		OnClose();
	}
}

SOCKET RIOSocket::GetRawSocket() const
{
	return rawSocket.load();
}

std::shared_ptr<RIOSocket> RIOSocket::PopFromSelfContainer()
{
	std::lock_guard<std::mutex> lock(requestLock);
	if (selfContainer.empty())
		return nullptr;

	auto pSocket = selfContainer.front();
	selfContainer.pop_front();

	return pSocket;
}

}