#pragma once

enum class RequestType
{
	RIO_READ,
	RIO_WRITE
};

struct RIOBuffer : public RIO_BUF
{
	char* RawBuf = nullptr;
	int Size = 0;
	RequestType Type;
};