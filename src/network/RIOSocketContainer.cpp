#include "stdafx.h"
#include "RIOSocketContainer.h"
#include "RIOSocket.h"

RIOSocketContainer::~RIOSocketContainer()
{
	std::unique_lock<std::mutex> lock(ContainerMutex);
	auto temp = std::move(Sockets);
	lock.unlock();

	for (auto& socket : temp)
		socket.second->DecreaseRef();
}

void RIOSocketContainer::AddSocket(RIOSocket* socket)
{
	auto rawSocket = socket->GetRawSocket();
	socket->IncreaseRef();

	std::lock_guard<std::mutex> lock(ContainerMutex);
	Sockets.insert(std::make_pair(rawSocket, socket));
}

void RIOSocketContainer::DeleteSocket(RIOSocket* socket)
{
	auto rawSocket = socket->GetRawSocket();

	std::lock_guard<std::mutex> lock(ContainerMutex);
	auto iter = Sockets.find(rawSocket);
	if (iter != Sockets.end())
	{
		auto* socket = iter->second;
		Sockets.erase(iter);
		socket->DecreaseRef();
	}
}