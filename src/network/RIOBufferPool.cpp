#include "stdafx.h"
#include "RIOBufferPool.h"
#include "RIOBuffer.h"

namespace network
{
RIOBufferPool* RIOBufferPool::Instance = nullptr;
std::mutex RIOBufferPool::InstMutex;

RIOBufferPool* RIOBufferPool::GetInstance()
{
	if (!Instance)
	{
		std::lock_guard<std::mutex> lock(InstMutex);
		if (!Instance)
			Instance = new RIOBufferPool();
	}

	return Instance;
}

RIOBufferPool::RIOBufferPool()
	: AllocIndex(0)
	, FreeIndex(0)
	, SlotList(16)
{
}

RIOBuffer* RIOBufferPool::AllocBuffer()
{
	auto index = AllocIndex.fetch_add(1) % SlotList.size();
	return SlotList[index].Alloc();
}

void RIOBufferPool::FreeBuffer(RIOBuffer* buffer)
{
	auto index = FreeIndex.fetch_add(1) % SlotList.size();
	SlotList[index].Free(buffer);
}

RIOBuffer* RIOBufferPool::Slot::Alloc()
{
	std::lock_guard<std::mutex> lock(ListMutex);
	RIOBuffer* buffer = nullptr;

	if (BufferList.empty())
	{
		auto newList = NewAlloc();
		BufferList.insert(std::end(BufferList), std::begin(newList), std::end(newList));
		buffer = BufferList.front();
		BufferList.pop_front();
	}
	else
	{
		buffer = BufferList.front();
		BufferList.pop_front();
	}

	return buffer;
}

void RIOBufferPool::Slot::Free(RIOBuffer* buffer)
{
	buffer->Length = 0;
	buffer->Offset = 0;

	std::lock_guard<std::mutex> lock(ListMutex);
	BufferList.push_back(buffer);
}

std::list<RIOBuffer*> RIOBufferPool::Slot::NewAlloc()
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
		buffer->RawBuf = base;

		list.push_back(buffer);
	}

	return list;
}
}