#pragma once

class RIOSocket;

class RIOSocketContainer
{
public:
	~RIOSocketContainer();

	void AddSocket(RIOSocket* socket);
	void DeleteSocket(RIOSocket* socket);

private:
	std::mutex ContainerMutex;
	std::unordered_map<SOCKET, RIOSocket*> Sockets;
};