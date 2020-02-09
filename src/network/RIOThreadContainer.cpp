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

void RIOThreadContainer::PollingThreadContainer::StartWorkerThread(int thread_count, int max_conn)
{
	if (IsRunning())
		return;

	auto completion_queue_size = RIOStatic::CalculateRIOCompletionQueueSize(max_conn);

	for (int i = 0; i < thread_count; ++i)
	{
		auto cq = RIOStatic::RIOFunc()->RIOCreateCompletionQueue(
			completion_queue_size, NULL);
		if (cq == RIO_INVALID_CQ)
		{
			RIOStatic::PrintConsole(
				std::string("create rio cq fail code : ") + std::to_string(WSAGetLastError()));
			continue;
		}

		ThreadSlot slot;
		slot.rio_cq_ = cq;
		auto slot_mutex = std::make_shared<std::mutex>();
		slot.mutex_ = slot_mutex;
		slot.thread_ = std::make_shared<std::thread>([this, cq, slot_mutex, completion_queue_size]()
		{
			WorkerThread(cq, slot_mutex, completion_queue_size);
		});

		slot_.push_back(std::move(slot));
	}

	if (!slot_.empty())
		stop_ = false;
}

void RIOThreadContainer::PollingThreadContainer::WorkerThread(RIO_CQ cq,
	std::shared_ptr<std::mutex> mutex, int completion_queue_size)
{
	RIORESULT* result = new RIORESULT[completion_queue_size];

	while (!stop_)
	{
		std::unique_lock<std::mutex> lock(*mutex);
		int result_size = RIOStatic::RIOFunc()->RIODequeueCompletion(cq, result,
			completion_queue_size);
		lock.unlock();

		DoIOCallBack(result, result_size);

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete[] result;
}

RIO_RQ RIOThreadContainer::PollingThreadContainer::BindSocket(const std::shared_ptr<RIOSocket>& socket)
{
	if (!IsRunning())
	{
		RIOStatic::PrintConsole("worker not running bind socket fail");
		return RIO_INVALID_RQ;
	}

	auto index = round_robin_++ % slot_.size();

	std::unique_lock<std::mutex> lock(*slot_[index].mutex_);
	auto rq = RIOStatic::RIOFunc()->RIOCreateRequestQueue(socket->GetRawSocket(),
		RIOStatic::RIO_MAX_OUTSTANDING_READ, 1, RIOStatic::RIO_MAX_OUTSTANDING_WRITE, 1,
		slot_[index].rio_cq_, slot_[index].rio_cq_, socket.get());
	lock.unlock();

	if (rq == RIO_INVALID_RQ)
		RIOStatic::PrintConsole("create request queue fail, rq invalid");

	return rq;
}

void RIOThreadContainer::PollingThreadContainer::Stop()
{
	if (!IsRunning())
		return;

	stop_ = true;

	for (const auto& t : slot_)
	{
		RIOStatic::RIOFunc()->RIOCloseCompletionQueue(t.rio_cq_);

		if (t.thread_->joinable())
			t.thread_->join();
	}
}

RIOThreadContainer::IOCPThreadContainer::~IOCPThreadContainer()
{
	Stop();
}

void RIOThreadContainer::IOCPThreadContainer::StartWorkThread(int thread_count, int max_conn)
{
	if (IsRunning())
		return;

	iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (iocp_ == NULL)
	{
		RIOStatic::PrintConsole(
			std::string("create io completion port fail ") + std::to_string(WSAGetLastError()));
		return;
	}

	auto completion_queue_size = RIOStatic::CalculateRIOCompletionQueueSize(max_conn);

	overlapped_ = new OVERLAPPED{};
	RIO_NOTIFICATION_COMPLETION noti_completion;
	noti_completion.Iocp.IocpHandle = iocp_;
	noti_completion.Iocp.CompletionKey = reinterpret_cast<void*>(1);
	noti_completion.Iocp.Overlapped = overlapped_;

	rio_cq_ = RIOStatic::RIOFunc()->RIOCreateCompletionQueue(
		completion_queue_size, &noti_completion);
	if (rio_cq_ == RIO_INVALID_CQ)
	{
		RIOStatic::PrintConsole(
			std::string("create rio completion queue fail " + std::to_string(WSAGetLastError())));
		return;
	}

	RIOStatic::RIOFunc()->RIONotify(rio_cq_);

	for (int i = 0; i < thread_count; ++i)
	{
		thread_.push_back(std::thread([this, completion_queue_size]()
		{
			WorkerThread(completion_queue_size);
		}));
	}

	if (!thread_.empty())
		stop_ = false;
}

void RIOThreadContainer::IOCPThreadContainer::WorkerThread(int completion_queue_size)
{
	RIORESULT* result = new RIORESULT[completion_queue_size];
	DWORD transferred = 0;
	void* completion_key = nullptr;
	OVERLAPPED* overlapped = nullptr;

	for (;;)
	{
		bool iocp_result = GetQueuedCompletionStatus(iocp_, &transferred,
			(PULONG_PTR)&completion_key, (LPOVERLAPPED*)&overlapped, INFINITE);

		if (!iocp_result)
		{
			RIOStatic::PrintConsole(
				std::string("iocp result false error ") + std::to_string(WSAGetLastError()));
			continue;
		}

		if (!completion_key)
			break;

		std::unique_lock<std::mutex> lock(mutex_);
		int result_size = RIOStatic::RIOFunc()->RIODequeueCompletion(rio_cq_, result,
			completion_queue_size);
		RIOStatic::RIOFunc()->RIONotify(rio_cq_);
		lock.unlock();

		DoIOCallBack(result, result_size);
	}

	delete[] result;
}

RIO_RQ RIOThreadContainer::IOCPThreadContainer::BindSocket(const std::shared_ptr<RIOSocket>& socket)
{
	if (!IsRunning())
	{
		RIOStatic::PrintConsole("worker not running bind socket fail");
		return RIO_INVALID_RQ;
	}

	std::unique_lock<std::mutex> lock(mutex_);
	auto rq = RIOStatic::RIOFunc()->RIOCreateRequestQueue(socket->GetRawSocket(),
		RIOStatic::RIO_MAX_OUTSTANDING_READ, 1, RIOStatic::RIO_MAX_OUTSTANDING_WRITE, 1,
		rio_cq_, rio_cq_, socket.get());
	lock.unlock();

	if (rq == RIO_INVALID_RQ)
		RIOStatic::PrintConsole(std::string("rio create request queue fail error ") +
			std::to_string(WSAGetLastError()));

	return rq;
}

void RIOThreadContainer::IOCPThreadContainer::Stop()
{
	if (!IsRunning())
		return;

	stop_ = true;

	for (int i = 0; i < thread_.size(); ++i)
		PostQueuedCompletionStatus(iocp_, 0, 0, overlapped_);

	for (auto& t : thread_)
	{
		if (t.joinable())
			t.join();
	}

	RIOStatic::RIOFunc()->RIOCloseCompletionQueue(rio_cq_);

	delete overlapped_;
}

void RIOThreadContainer::StartPollingThread(int thread_count, int max_conn)
{
	if (polling_container_.IsRunning())
		return;

	polling_container_.StartWorkerThread(thread_count, max_conn);
}

void RIOThreadContainer::StartIOCPThread(int thread_count, int max_conn)
{
	if (iocp_container_.IsRunning())
		return;

	iocp_container_.StartWorkThread(thread_count, max_conn);
}

void RIOThreadContainer::Stop()
{
	if (polling_container_.IsRunning())
		polling_container_.Stop();

	if (iocp_container_.IsRunning())
		iocp_container_.Stop();
}

RIO_RQ RIOThreadContainer::BindSocket(const std::shared_ptr<RIOSocket>& socket)
{
	if (polling_container_.IsRunning() && !iocp_container_.IsRunning())
		return polling_container_.BindSocket(socket);
	
	if (!polling_container_.IsRunning() && iocp_container_.IsRunning())
		return iocp_container_.BindSocket(socket);

	if (polling_container_.IsRunning() && iocp_container_.IsRunning())
	{
		static uint64_t round_robin = 0;

		if (round_robin++ % 2)
			return polling_container_.BindSocket(socket);
		else
			return iocp_container_.BindSocket(socket);
	}

	return RIO_INVALID_RQ;
}

void DoIOCallBack(RIORESULT* result, int size)
{
	for (int i = 0; i < size; ++i)
	{
		auto status = result[i].Status;
		auto transferred = result[i].BytesTransferred;
		auto socket = reinterpret_cast<RIOSocket*>(result[i].SocketContext);
		auto buf = reinterpret_cast<RIOBuffer*>(result[i].RequestContext);

		auto shared_socket = socket->PopFromSelfContainer();
		shared_socket->OnIOCallBack(status, transferred, buf);
	}
}
}
