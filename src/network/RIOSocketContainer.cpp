#include "stdafx.h"
#include "RIOSocketContainer.h"
#include "RIOSocket.h"

namespace network
{
RIOSocketContainer::~RIOSocketContainer()
{
	std::unique_lock<std::mutex> lock(lock);
	auto temp = std::move(sockets);
	lock.unlock();

	for (auto& socket : temp)
		socket->DecRef();
}

void RIOSocketContainer::AddSocket(RIOSocket* socket)
{
	socket->IncRef();

	std::lock_guard<std::mutex> lock(lock);
	sockets.insert(socket);
}

void RIOSocketContainer::DeleteSocket(RIOSocket* socket)
{
	std::lock_guard<std::mutex> lock(lock);
	auto iter = sockets.find(socket);
	if (iter != sockets.end())
	{
		auto* socket = *iter;
		sockets.erase(iter);
		socket->DecRef();
	}
}

std::set<RIOSocket*> RIOSocketContainer::GetAll()
{
	std::lock_guard<std::mutex> lock(lock);
	return sockets;
}
}