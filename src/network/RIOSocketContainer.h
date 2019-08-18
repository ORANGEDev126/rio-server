#pragma once

namespace network { class RIOSocket; }

namespace network
{
class RIOSocketContainer
{
public:
	~RIOSocketContainer();

	void AddSocket(const std::shared_ptr<RIOSocket>& socket);
	void DeleteSocket(const std::shared_ptr<RIOSocket>& socket);
	std::set<std::shared_ptr<RIOSocket>> GetAll();

private:
	std::mutex lock;
	std::set<std::shared_ptr<RIOSocket>> sockets;
};
}