#include "stdafx.h"
#include "RIOBufferPool.h"
#include "RIOBuffer.h"
#include "RIOStatic.h"

namespace network
{

std::shared_ptr<RIOBuffer> RIOBufferPool::Alloc()
{
	auto index = allocCount_.fetch_add(1) % slots_.size();
	auto* buf = slots_[index].Alloc();
	return std::shared_ptr<RIOBuffer>(buf, [](auto buf)
	{
		RIOBufferPool::GetInstance()->Free(buf);
	});
}

void RIOBufferPool::Free(RIOBuffer* buffer)
{
	auto index = freeCount_.fetch_add(1) % slots_.size();
	slots_[index].Free(buffer);
}

RIOBuffer* RIOBufferPool::Slot::Alloc()
{
	std::lock_guard<std::mutex> lock(mutex_);
	RIOBuffer* buffer = nullptr;

	if (buf_.empty())
	{
		auto newBuf = New();
		buf_ = std::move(newBuf);
	}

	buffer = buf_.front();
	buf_.pop_front();

	return buffer;
}

void RIOBufferPool::Slot::Free(RIOBuffer* buffer)
{
	buffer->Reset();

	std::lock_guard<std::mutex> lock(mutex_);
	buf_.push_front(buffer);
}

std::list<RIOBuffer*> RIOBufferPool::Slot::New()
{
	std::list<RIOBuffer*> list;

	char* base = reinterpret_cast<char*>(VirtualAlloc(NULL, Static::RIO_BUFFER_GRANULARITY,
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

	if (!base)
	{
		auto error = GetLastError();
		Static::PrintConsole(std::string("virtual alloc return nullptr : ") + std::to_string(error));
		return {};
	}

	for (int i = 0; i < Static::RIO_BUFFER_GRANULARITY / Static::RIO_BUFFER_SIZE; ++i)
	{
		base += i * Static::RIO_BUFFER_SIZE;
		RIO_BUFFERID buffer_id = Static::RIOFunc()->RIORegisterBuffer(base, Static::RIO_BUFFER_SIZE);

		RIOBuffer* buf = new RIOBuffer(buffer_id, base);
		list.push_back(buf);
	}

	return list;
}
}