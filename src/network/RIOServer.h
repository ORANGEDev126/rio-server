#pragma once

#include "RIOSocket.h"

namespace network { class RIOSocketContainer; }
namespace network { class RIOThreadContainer; }

namespace network
{
class RIOServer
{
public:
	~RIOServer();

	void Run(int port, int thread_count, int max_conn);
	void Stop();

private:
	void AcceptLoop();

	std::function<std::shared_ptr<RIOSocket>()> sock_allocator_;
	SOCKET listen_sock_;
	int port_;
	bool stop_;
	std::thread accept_thread_;
	std::shared_ptr<RIOSocketContainer> socket_container_;
	std::shared_ptr<RIOThreadContainer> thread_container_;
};
}