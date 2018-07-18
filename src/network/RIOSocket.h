#pragma once

#include "ReferenceObject.h"

namespace network { struct RIOBuffer; }
namespace network { class RIOServer; }
namespace network { class RIOBuffer; }

namespace network
{
class RIOSocket : public ReferenceObject
{
public:
	RIOSocket(SOCKET rawSock, const SOCKADDR_IN& addr);
	virtual ~RIOSocket() = default;

	void Initialize(RIO_RQ requestQueue, RIOServer* server);
	void OnIOCallBack(int status, RIOBuffer* buffer, int transferred);
	void Read();
	void Write(RIOBuffer* buffer);
	void Close();
	SOCKET GetRawSocket() const;

private:
	virtual void OnRead(RIOBuffer* buffer, int transferred) = 0;
	virtual void OnWrite(RIOBuffer* buffer, int transferred) = 0;
	virtual void OnConnected() = 0;
	virtual void OnClose() = 0;

	std::atomic<SOCKET> RawSocket;
	SOCKADDR_IN Addr;
	RIO_RQ RequestQueue;
	RIOBuffer* ReadBuffer;
	RIOServer* Server;
};
}
