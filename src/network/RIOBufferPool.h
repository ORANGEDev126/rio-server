#pragma once

namespace network { struct RIOBuffer; }

namespace network
{
class RIOBufferPool
{
public:
	static RIOBufferPool* GetInstance()
	{
		static RIOBufferPool pool;
		return &pool;
	}

	class Slot
	{
	public:
		RIOBuffer * Alloc();
		void Free(RIOBuffer* buffer);

	private:
		std::list<RIOBuffer*> New();

		std::list<RIOBuffer*> buf_;
		std::mutex mutex_;
	};

	RIOBufferPool();

	std::shared_ptr<RIOBuffer> Alloc();
	
private:
	void Free(RIOBuffer* buffer);

	std::vector<Slot> slots_;
	std::atomic_int64_t allocCount_;
	std::atomic_int64_t freeCount_;
};
}
