#pragma once

#include "RIOSocket.h"

namespace network { class RIOSocketContainer; }

namespace network
{
class TestClientManager
{
public:
	class TestClientSocket : public RIOSocket
	{
	private:
		virtual void OnConnected() override;
		virtual void OnDisconnected(CloseReason reason, std::string str) override;
		virtual void OnRead(std::istream& packet) override;

		std::mutex mutex_;
		std::list<std::vector<char>> message_;
	};

	void StartTest(int conn_thread_count,
		int dis_thread_count, int send_thread_count);

private:
	std::shared_ptr<RIOSocketContainer> container_;
};
}