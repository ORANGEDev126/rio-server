#include "stdafx.h"
#include "RIOBufferPool.h"
#include "RIOBuffer.h"

namespace network
{
RIOBufferPool* RIOBufferPool::instance = nullptr;
std::mutex RIOBufferPool::instLock;

RIOBufferPool* RIOBufferPool::GetInstance()
{
	if (!instance)
	{
		std::lock_guard<std::mutex> lock(instLock);
		if (!instance)
			instance = new RIOBufferPool();
	}

	return instance;
}

RIOBufferPool::RIOBufferPool()
	: allocCount(0)
	, freeCount(0)
	, slots(16)
{
}

RIOBuffer* RIOBufferPool::Alloc()
{
	auto index = allocCount.fetch_add(1) % slots.size();
	return slots[index].Alloc();
}

void RIOBufferPool::Free(RIOBuffer* buffer)
{
	auto index = freeCount.fetch_add(1) % slots.size();
	slots[index].Free(buffer);
}

RIOBuffer* RIOBufferPool::Slot::Alloc()
{
	std::lock_guard<std::mutex> lock(mutex);
	RIOBuffer* buffer = nullptr;

	if (buf.empty())
	{
		auto newBuf = New();
		buf.insert(std::end(buf), std::begin(newBuf), std::end(newBuf));
		buffer = buf.front();
		buf.pop_front();
	}
	else
	{
		buffer = buf.front();
		buf.pop_front();
	}

	return buffer;
}

void RIOBufferPool::Slot::Free(RIOBuffer* buffer)
{
	buffer->Length = 0;
	buffer->Offset = 0;
	buffer->size = 0;

	std::lock_guard<std::mutex> lock(mutex);
	buf.push_front(buffer);
}

std::list<RIOBuffer*> RIOBufferPool::Slot::New()
{
	std::list<RIOBuffer*> list;

	char* base = reinterpret_cast<char*>(VirtualAlloc(NULL, GRANULARITY, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	if (!base)
	{
		auto error = GetLastError();
		PrintConsole(std::string("virtual alloc return nullptr : ") + std::to_string(error));
		return {};
	}

	for (int i = 0; i < GRANULARITY / BUFFER_SIZE; ++i)
	{
		base += i * BUFFER_SIZE;
		RIO_BUFFERID bufferID = g_RIO.RIORegisterBuffer(base, BUFFER_SIZE);

		RIOBuffer* buffer = new RIOBuffer();
		buffer->BufferId = bufferID;
		buffer->Length = 0;
		buffer->Offset = 0;
		buffer->rawBuf = base;

		list.push_back(buffer);
	}

	return list;
}
}