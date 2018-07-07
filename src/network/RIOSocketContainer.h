#pragma once

class RIOSocket;

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