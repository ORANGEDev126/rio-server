#pragma once

class RIOSocket;

class RIOSocketContainer
{
public:
	~RIOSocketContainer();

	void AddSocket(RIOSocket* socket);
	void DeleteSocket(RIOSocket* socket);
	std::unordered_map<SOCKET, RIOSocket*> GetAll();

private:
	std::mutex ContainerMutex;
	std::unordered_map<SOCKET, RIOSocket*> Sockets;
};