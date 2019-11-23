#include "stdafx.h"
#include "RIOStatic.h"
#include "RIOBuffer.h"

namespace network
{
RIOBuffer::RIOBuffer(RIO_BUFFERID buf_id, char* raw_buf)
	: RIO_BUF{ buf_id, 0, 0 }, raw_buf_(raw_buf)
{

}

void RIOBuffer::PrepareRead()
{
	type_ = READ;
	Offset = size_;
	Length = Static::RIO_BUFFER_SIZE - size_;
}

void RIOBuffer::PrepareWrite()
{
	type_ = WRITE;
	Offset = 0;
	Length = size_;
}

void RIOBuffer::Reset()
{
	Offset = 0;
	Length = 0;
	size_ = 0;
}
}