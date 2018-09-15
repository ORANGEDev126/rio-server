#pragma once

namespace network
{
enum class REQUEST_TYPE
{
	RIO_READ,
	RIO_WRITE
};

struct RIOBuffer : public RIO_BUF
{
	char* rawBuf = nullptr;
	int size = 0;
	REQUEST_TYPE type;
};
}