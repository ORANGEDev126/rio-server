#pragma once

namespace network { class RIOSocket; }

namespace network
{
class RIOSocketContainer
{
public:
	~RIOSocketContainer();

	void AddSocket(RIOSocket* socket);
	void DeleteSocket(RIOSocket* socket);
	std::set<RIOSocket*> GetAll();

private:
	std::mutex ContainerMutex;
	std::set<RIOSocket*> Sockets;
};
}