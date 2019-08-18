#pragma once

namespace network { class RIOSocket; }

namespace network
{
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