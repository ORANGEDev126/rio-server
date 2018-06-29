#pragma once

class RIOSocket;

struct ThreadSlot
{
	std::shared_ptr<std::thread> Thread;
	RIO_CQ RIOCQ;
	int64_t BindedCount;
};

class RIOThreadContainer
{
public:
	RIOThreadContainer(int threadCount);
	~RIOThreadContainer();

	void WorkerThread(RIO_CQ cq);
	void StartThread();
	void StopThread();
	RIO_RQ BindSocket(RIOSocket* socket);

private:
	std::vector<ThreadSlot> Slots;
	std::mutex SlotMutex;
	int ThreadCount;
	volatile bool StopFlag;
};