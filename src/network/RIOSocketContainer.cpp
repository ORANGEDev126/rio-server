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
		socket->Close();
}

void RIOSocketContainer::AddSocket(const std::shared_ptr<RIOSocket>& socket)
{
	std::lock_guard<std::mutex> lock(lock);
	sockets.insert(socket);
}

void RIOSocketContainer::DeleteSocket(const std::shared_ptr<RIOSocket>& socket)
{
	std::lock_guard<std::mutex> lock(lock);
	sockets.erase(socket);
}

std::set<std::shared_ptr<RIOSocket>> RIOSocketContainer::GetAll()
{
	std::lock_guard<std::mutex> lock(lock);
	return sockets;
}
}