#pragma once

namespace network
{
enum class RequestType
{
	RIO_READ,
	RIO_WRITE
};

struct RIOBuffer : public RIO_BUF
{
	char* RawBuf = nullptr;
	RequestType Type;
};
}