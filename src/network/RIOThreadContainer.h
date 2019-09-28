#pragma once

namespace network { class RIOSocket; }

namespace network
{

static constexpr int MAX_COMPLETION_QUEUE_SIZE = 32 * 1024;

struct ThreadSlot
{
	std::shared_ptr<std::thread> thread;
	RIO_CQ RIOCQ;
	int64_t bindedCount;
};

class RIOThreadContainer
{
public:
	RIOThreadContainer(int threadCount);
	~RIOThreadContainer();

	void WorkerThread(RIO_CQ cq);
	void StartThread();
	void StopThread();
	RIO_RQ BindSocket(SOCKET rawSock, const std::shared_ptr<RIOSocket>& socket);

private:
	std::vector<ThreadSlot> slots;
	std::mutex slotLock;
	int threadCount;
	volatile bool stop;
};
}