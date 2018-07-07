#include "stdafx.h"
#include "RIOSocketContainer.h"
#include "RIOSocket.h"

RIOSocketContainer::~RIOSocketContainer()
{
	std::unique_lock<std::mutex> lock(ContainerMutex);
	auto temp = std::move(Sockets);
	lock.unlock();

	for (auto& socket : temp)
		socket->DecRef();
}

void RIOSocketContainer::AddSocket(RIOSocket* socket)
{
	socket->IncRef();

	std::lock_guard<std::mutex> lock(ContainerMutex);
	Sockets.insert(socket);
}

void RIOSocketContainer::DeleteSocket(RIOSocket* socket)
{
	std::lock_guard<std::mutex> lock(ContainerMutex);
	auto iter = Sockets.find(socket);
	if (iter != Sockets.end())
	{
		auto* socket = *iter;
		Sockets.erase(iter);
		socket->DecRef();
	}
}

std::set<RIOSocket*> RIOSocketContainer::GetAll()
{
	std::lock_guard<std::mutex> lock(ContainerMutex);
	return Sockets;
}