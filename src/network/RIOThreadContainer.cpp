#include "stdafx.h"
#include "RIOThreadContainer.h"
#include "RIOSocket.h"
#include "RIOBuffer.h"
#include "RIOStatic.h"

namespace network
{

RIOThreadContainer::PollingThreadContainer::~PollingThreadContainer()
{
	Stop();
}

void RIOThreadContainer::PollingThreadContainer::StartWorkerThread(int thread_count)
{
	if (IsRunning())
		return;

	stop_ = false;

	for (int i = 0; i < thread_count; ++i)
	{
		auto cq = Static::RIOFunc()->RIOCreateCompletionQueue(
			Static::RIO_MAX_COMPLETION_QUEUE_SIZE, NULL);
		if (cq == RIO_INVALID_CQ)
		{
			Static::PrintConsole(
				std::string("create rio cq fail code : ") + std::to_string(WSAGetLastError));
			continue;
		}

		ThreadSlot slot;
		slot.cq_ = cq;
		slot.mutex_ = std::make_shared<std::mutex>();
		slot.thread_ = std::make_shared<std::thread>([slot.cq_, slot.mutex_, this]()
		{
			WorkerThread(slot.cq_, slot.mutex_);
		});

		slots_.push_back(std::move(slot));
	}
}

void RIOThreadContainer::PollingThreadContainer::WorkerThread(RIO_CQ cq,
	std::shared_ptr<std::mutex> mutex)
{
	RIORESULT* result = new RIORESULT[Static::RIO_MAX_COMPLETION_QUEUE_SIZE];

	while (!stop_)
	{
		std::unique_lock<std::mutex> lock(*mutex);
		int result_size = Static::RIOFunc()->RIODequeueCompletion(cq, result,
			Static::RIO_MAX_COMPLETION_QUEUE_SIZE);
		lock.unlock();

		DoIOCallBack(result, result_size);

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete result;
}

RIO_RQ RIOThreadContainer::PollingThreadContainer::BindSocket(const std::shared_ptr<RIOSocket>& socket)
{
	if (slots_.empty())
	{
		Static::PrintConsole("slot is empty, rio bind socket fail");
		return RIO_INVALID_RQ;
	}

	int index = round_robin_++ % slots_.size();

	std::unique_lock<std::mutex> lock(*slots_[index].mutex_);
	auto rq = Static::RIOFunc()->RIOCreateRequestQueue(socket->GetRawSocket(),
		Static::RIO_MAX_OUTSTANDING_READ, 1, Static::RIO_MAX_OUTSTANDING_WRITE, 1,
		slots_[index].cq, slots_[index].cq, socket.get());
	lock.unlock();

	if (rq == RIO_INVALID_RQ)
		Static::PrintConsole("create request queue fail, rq invalid");

	return rq;
}

void RIOThreadContainer::PollingThreadContainer::Stop()
{
	if (!IsRunning())
		return;

	stop_ = true;

	for (const auto& t : slots_)
	{
		Static::RIOFunc()->RIOCloseCompletionQueue(t.cq_);

		if (t.thread_->joinable())
			t.thread_->join();
	}
}

void RIOThreadContainer::IOCPThreadContainer::~IOCPThreadContainer()
{
	Stop();
}

void RIOThreadContainer::IOCPThreadContainer::StartWorkThread(int thread_count)
{
	if (IsRunning())
		return;

	stop_ = false;

	iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (iocp_ == NULL)
	{
		Static::PrintConsole(
			std::string("create io completion port fail ") + std::to_string(WSAGetLastError()));
		return;
	}

	OVERLAPPED* overlapped = new OVERLAPPED{};
	RIO_NOTIFICATION_COMPLETION noti_completion;
	noti_completion.Iocp.IocpHandle = iocp_;
	noti_completion.Iocp.CompletionKey = reinterpret_cast<void*>(1);
	noti_completion.Iocp.Overlapped = overlapped;

	rio_cq_ = Static::RIOFunc()->RIOCreateCompletionQueue(
		Static::RIO_MAX_COMPLETION_QUEUE_SIZE, &noti_completion);
	if (rio_cq_ == RIO_INVALID_CQ)
	{
		Static::PrintConsole(
			std::string("create rio completion queue fail " + std::to_string(WSAGetLastError())));
		return;
	}

	Static::RIOFunc()->RIONotify(rio_cq_);

	for (int i = 0; i < thread_count; ++i)
	{
		thread_.push_back(std::thread([this]()
		{
			WorkerThread();
		}));
	}
}

void RIOThreadContainer::IOCPThreadContainer::WorkerThread()
{
	RIORESULT* result = new RIORESULT[Static::RIO_MAX_COMPLETION_QUEUE_SIZE];
	DWORD transferred = 0;
	void* completion_key = nullptr;
	OVERLAPPED* overlapped = nullptr;

	for (;;)
	{
		bool iocp_result = GetQueuedCompletionStatus(iocp_, &transferred,
			(PULONG_PTR)&completion_key, (LPOVERLAPPED*)&overlapped, INFINITE);

		if (!iocp_result)
		{
			Static::PrintConsole(
				std::string("iocp result false error ") + std::to_string(WSAGetLastError()));
			continue
		}

		if (!completion_key)
			break;

		std::unique_lock<std::mutex> lock(mutex_);
		int result_size = Static::RIOFunc()->RIODequeueCompletion(rio_cq_, result,
			Static::RIO_MAX_COMPLETION_QUEUE_SIZE);
		Static::RIOFunc()->RIONotify(rio_cq_);
		lock.unlock();

		DoIOCallBack(result, result_size);
	}

	delete result;
}

void RIOThreadContainer::WorkerThread(RIO_CQ cq)
{
	RIORESULT result[MAX_COMPLETION_QUEUE_SIZE] = { 0, };

	while (!stop)
	{
		int size = g_RIO.RIODequeueCompletion(cq, result, MAX_COMPLETION_QUEUE_SIZE);

		for (int i = 0; i < size; ++i)
		{
			auto status = result[i].Status;
			auto transferred = (result[i].BytesTransferred);
			auto socketContext = reinterpret_cast<RIOSocket*>(result[i].SocketContext);
			auto buffer = reinterpret_cast<RIOBuffer*>(result[i].RequestContext);

			auto socket = socketContext->PopFromSelfContainer();
			socket->OnIOCallBack(status, buffer, transferred);
		}

		std::this_thread::yield();
	}
}

void RIOThreadContainer::StartThread()
{
	stop = false;

	std::lock_guard<std::mutex> lock(slotLock);
	for (int i = 0; i < threadCount; ++i)
	{
		auto cq = g_RIO.RIOCreateCompletionQueue(MAX_COMPLETION_QUEUE_SIZE, NULL);
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

RIO_RQ RIOThreadContainer::BindSocket(SOCKET rawSock, const std::shared_ptr<RIOSocket>& socket)
{
	std::lock_guard<std::mutex> lock(slotLock);
	if (slots.empty())
	{
		PrintConsole("bind socket error slots empty");
		return RIO_INVALID_RQ;
	}

	int slotIndex = static_cast<int>(rawSock) % slots.size();

	auto rq = g_RIO.RIOCreateRequestQueue(rawSock, 1, 1, MAX_OUTSTANDING_SEND_SIZE, 1,
		slots[slotIndex].RIOCQ, slots[slotIndex].RIOCQ, socket.get());
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
