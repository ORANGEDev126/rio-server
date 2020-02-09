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
		~Slot();
		RIOBuffer * Alloc();
		void Free(RIOBuffer* buffer);

	private:
		std::list<RIOBuffer*> New();

		std::list<RIOBuffer*> buf_;
		std::vector<char*> alloc_page_;
		std::vector<RIO_BUFFERID> alloc_buffer_id_;
		std::mutex mutex_;
	};

	std::shared_ptr<RIOBuffer> Alloc();
	
private:
	RIOBufferPool() = default;

	void Free(RIOBuffer* buffer);

	std::vector<Slot> slots_{ 16 };
	std::atomic_int64_t allocCount_{ 0 };
	std::atomic_int64_t freeCount_{ 0 };
};
}
