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
	char* GetRawBuf() const { return raw_buf_; }
	void PrepareRead();
	void PrepareWrite();
	void Reset();
	bool IsReadRequest() const { return type_ == READ; }
	bool IsWriteRequest() const { return type_ == WRITE; }
	bool IsFull() const { return size_ == Static::RIO_BUFFER_SIZE; }

private:
	char* raw_buf_{ nullptr };
	int size_{ 0 };
	RequestType type_
};
}