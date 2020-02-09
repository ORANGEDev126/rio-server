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

RIOBufferPool::Slot::~Slot()
{
	std::lock_guard<std::mutex> lock(mutex_);

	for (const auto& id : alloc_buffer_id_)
		RIOStatic::RIOFunc()->RIODeregisterBuffer(id);

	for (const auto& page : alloc_page_)
		VirtualFree(page, RIOStatic::RIO_BUFFER_GRANULARITY, MEM_RELEASE);
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

	char* base = reinterpret_cast<char*>(VirtualAlloc(NULL, RIOStatic::RIO_BUFFER_GRANULARITY,
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

	if (!base)
	{
		auto error = GetLastError();
		RIOStatic::PrintConsole(std::string("virtual alloc return nullptr : ") + std::to_string(error));
		return {};
	}

	alloc_page_.push_back(base);

	for (int i = 0; i < RIOStatic::RIO_BUFFER_GRANULARITY / RIOStatic::RIO_BUFFER_SIZE; ++i)
	{
		base += i * RIOStatic::RIO_BUFFER_SIZE;
		RIO_BUFFERID buffer_id = RIOStatic::RIOFunc()->RIORegisterBuffer(base, RIOStatic::RIO_BUFFER_SIZE);

		RIOBuffer* buf = new RIOBuffer(buffer_id, base);
		list.push_back(buf);
		alloc_buffer_id_.push_back(buffer_id);
	}

	return list;
}
}