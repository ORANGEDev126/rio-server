#pragma once

struct RIOBuffer;

class RIOBufferPool
{
public:
	static RIOBufferPool* GetInstance();

	class Slot
	{
	public:
		RIOBuffer* Alloc();
		void Free(RIOBuffer* buffer);

	private:
		std::list<RIOBuffer*> NewAlloc();

		std::list<RIOBuffer*> BufferList;
		std::mutex ListMutex;
	};

	RIOBufferPool();
	~RIOBufferPool();

	RIOBuffer* AllocBuffer();
	void FreeBuffer(RIOBuffer* buffer);

private:
	static RIOBufferPool* Instance;
	static std::mutex InstMutex;

	std::vector<Slot> SlotList;
	std::atomic_int64_t AllocIndex;
	std::atomic_int64_t FreeIndex;
};

#define g_RIOBufferPool RIOBufferPool::GetInstance()
#define GRANULARITY 65536
#define BUFFER_SIZE 8192