#include "main/stdafx.h"

#include "RIOThreadContainer.h"
#include "RIOSocket.h"
#include "RIOBuffer.h"

RIOThreadContainer::RIOThreadContainer(int threadCount)
	: ThreadCount(threadCount)
	, StopFlag(false)
{
	StartThread();
}

RIOThreadContainer::~RIOThreadContainer()
{
	StopThread();
}

void RIOThreadContainer::WorkerThread(RIO_CQ cq)
{
	RIORESULT result[256] = { 0, };

	while (!StopFlag)
	{
		auto size = g_RIO.RIODequeueCompletion(cq, result, 256);

		for (int i = 0; i < size; ++i)
		{
			if (result[i].Status)
			{
				std::cout << "dequeue completion status error " << result[i].Status << std::endl;
				continue;
			}

			auto transferred =(result[i].BytesTransferred);
			auto socketContext = reinterpret_cast<RIOSocket*>(result[i].SocketContext);
			auto requestContext = reinterpret_cast<RIOBuffer*>(result[i].RequestContext);

			socketContext->OnIOCallBack(requestContext, transferred);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void RIOThreadContainer::StartThread()
{
	StopFlag = false;

	std::lock_guard<std::mutex> lock(SlotMutex);
	for (int i = 0; i < ThreadCount; ++i)
	{
		auto cq = g_RIO.RIOCreateCompletionQueue(256, NULL);
		auto thread = std::make_shared<std::thread>([this, &cq]()
		{
			WorkerThread(cq);
		});
		
		Slots.emplace_back(thread, cq, 0);
	}
}

void RIOThreadContainer::StopThread()
{
	StopFlag = true;

	std::lock_guard<std::mutex> lock(SlotMutex);
	for (int i = 0; i < Slots.size(); ++i)
	{
		Slots[i].Thread->join();
	}

	Slots.clear();
}

RIO_RQ RIOThreadContainer::BindSocket(SOCKET sock, RIOSocket* socketContext)
{
	std::lock_guard<std::mutex> lock(SlotMutex);
	if (Slots.empty())
	{
		std::cout << "bind socket error slots empty" << std::endl;
		return RIO_INVALID_RQ;
	}

	int minIndex = 0;
	for (int i = 0; i < Slots.size(); ++i)
	{
		if (Slots[i].BindedCount < Slots[minIndex].BindedCount)
			minIndex = i;
	}

	auto rq = g_RIO.RIOCreateRequestQueue(sock, 1, 1, 1, 1,
		Slots[minIndex].RIOCQ, Slots[minIndex].RIOCQ, socketContext);
	if (rq == RIO_INVALID_RQ)
	{
		auto error = WSAGetLastError();
		std::cout << "create request error : " << error << std::endl;
	}

	return rq;
}