#include "stdafx.h"

#include "RIOThreadContainer.h"
#include "RIOSocket.h"
#include "RIOBuffer.h"

RIOThreadContainer::RIOThreadContainer(int threadCount)
	: ThreadCount(threadCount)
	, StopFlag(false)
{
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
		int size = g_RIO.RIODequeueCompletion(cq, result, 256);

		for (int i = 0; i < size; ++i)
		{
			auto status = result[i].Status;
			auto transferred =(result[i].BytesTransferred);
			auto socketContext = reinterpret_cast<RIOSocket*>(result[i].SocketContext);
			auto requestContext = reinterpret_cast<RIOBuffer*>(result[i].RequestContext);

			socketContext->OnIOCallBack(status, requestContext, transferred);
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
		if (cq == RIO_INVALID_CQ)
		{
			auto error = WSAGetLastError();
			PrintConsole(std::string("invalid completion queue ") + std::to_string(error));
			continue;
		}

		auto thread = std::make_shared<std::thread>([this, cq]()
		{
			WorkerThread(cq);
		});
		
		ThreadSlot slot{ thread, cq, 0 };
		Slots.push_back(slot);
	}
}

void RIOThreadContainer::StopThread()
{
	StopFlag = true;

	std::lock_guard<std::mutex> lock(SlotMutex);
	for (int i = 0; i < Slots.size(); ++i)
	{
		Slots[i].Thread->join();
		g_RIO.RIOCloseCompletionQueue(Slots[i].RIOCQ);
	}

	Slots.clear();
}

RIO_RQ RIOThreadContainer::BindSocket(SOCKET rawSock, RIOSocket* socket)
{
	std::lock_guard<std::mutex> lock(SlotMutex);
	if (Slots.empty())
	{
		PrintConsole("bind socket error slots empty");
		return RIO_INVALID_RQ;
	}

	int slotIndex = static_cast<int>(rawSock) % Slots.size();

	auto rq = g_RIO.RIOCreateRequestQueue(rawSock, 1, 1, 1, 1,
		Slots[slotIndex].RIOCQ, Slots[slotIndex].RIOCQ, socket);
	if (rq == RIO_INVALID_RQ)
	{
		auto error = WSAGetLastError();
		PrintConsole(std::string("create request error : ") + std::to_string(error));
		return rq;
	}

	++Slots[slotIndex].BindedCount;
	return rq;
}
