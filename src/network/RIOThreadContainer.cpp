#include "stdafx.h"

#include "RIOThreadContainer.h"
#include "RIOSocket.h"
#include "RIOBuffer.h"

namespace network
{
RIOThreadContainer::RIOThreadContainer(int threadCount)
	: threadCount(threadCount)
	, stop(false)
{
}

RIOThreadContainer::~RIOThreadContainer()
{
	StopThread();
}

void RIOThreadContainer::WorkerThread(RIO_CQ cq)
{
	RIORESULT result[256] = { 0, };

	while (!stop)
	{
		int size = g_RIO.RIODequeueCompletion(cq, result, 256);

		for (int i = 0; i < size; ++i)
		{
			auto status = result[i].Status;
			auto transferred = (result[i].BytesTransferred);
			auto socketContext = reinterpret_cast<RIOSocket*>(result[i].SocketContext);
			auto buffer = reinterpret_cast<RIOBuffer*>(result[i].RequestContext);

			socketContext->OnIOCallBack(status, buffer, transferred);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void RIOThreadContainer::StartThread()
{
	stop = false;

	std::lock_guard<std::mutex> lock(slotLock);
	for (int i = 0; i < threadCount; ++i)
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
		slots.push_back(slot);
	}
}

void RIOThreadContainer::StopThread()
{
	stop = true;

	std::lock_guard<std::mutex> lock(slotLock);
	for (int i = 0; i < slots.size(); ++i)
	{
		slots[i].thread->join();
		g_RIO.RIOCloseCompletionQueue(slots[i].RIOCQ);
	}

	slots.clear();
}

RIO_RQ RIOThreadContainer::BindSocket(SOCKET rawSock, RIOSocket* socket)
{
	std::lock_guard<std::mutex> lock(slotLock);
	if (slots.empty())
	{
		PrintConsole("bind socket error slots empty");
		return RIO_INVALID_RQ;
	}

	int slotIndex = static_cast<int>(rawSock) % slots.size();

	auto rq = g_RIO.RIOCreateRequestQueue(rawSock, 1, 1, 1, 1,
		slots[slotIndex].RIOCQ, slots[slotIndex].RIOCQ, socket);
	if (rq == RIO_INVALID_RQ)
	{
		auto error = WSAGetLastError();
		PrintConsole(std::string("create request error : ") + std::to_string(error));
		return rq;
	}

	++slots[slotIndex].bindedCount;
	return rq;
}
}
