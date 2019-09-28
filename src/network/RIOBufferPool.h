#pragma once

#define GRANULARITY 65536
#define BUFFER_SIZE 8192

namespace network { struct RIOBuffer; }

namespace network
{
class RIOBufferPool
{
public:
	static RIOBufferPool* GetInstance();

	class Slot
	{
	public:
		RIOBuffer * Alloc();
		void Free(RIOBuffer* buffer);

	private:
		std::list<RIOBuffer*> New();

		std::list<RIOBuffer*> buf;
		std::mutex mutex;
	};

	RIOBufferPool();
	~RIOBufferPool();

	std::shared_ptr<RIOBuffer> Alloc();
	
private:
	void Free(RIOBuffer* buffer);

	static RIOBufferPool* instance;
	static std::mutex instLock;

	std::vector<Slot> slots;
	std::atomic_int64_t allocCount;
	std::atomic_int64_t freeCount;
};
}
