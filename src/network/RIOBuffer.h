#pragma once

namespace network
{
class RIOBuffer : public RIO_BUF
{
public:
	RIOBuffer(RIO_BUFFERID buf_id, const char* raw_buf);

	enum RequestType
	{
		READ,
		WRITE
	};

	int GetSize() const { return size_; }
	void SetSize(int size) { size_ = size; }
	void PrepareRead();
	void PrepareWrite();
	void Reset();

private:
	char* raw_buf_{ nullptr };
	int size_{ 0 };
	RequestType type_
};
}