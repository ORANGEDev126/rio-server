#pragma once

#include "ReferenceObject.h"

namespace network { struct RIOBuffer; }
namespace network { class RIOServer; }
namespace network { struct RIOBuffer; }

namespace network
{
class RIOSocket : public ReferenceObject
{
public:
	RIOSocket(SOCKET rawSock, const SOCKADDR_IN& addr);
	virtual ~RIOSocket() = default;

	void Initialize(RIO_RQ queue, RIOServer* owner);
	void OnIOCallBack(int status, RIOBuffer* buffer, int transferred);
	void Read();
	bool Write(RIOBuffer* buffer);
	void Close();
	SOCKET GetRawSocket() const;

private:
	void OnReadCallBack(RIOBuffer* buffer, int transferred);
	void OnWriteCallBack(RIOBuffer* buffer, int transferred);

	virtual void OnRead(std::istream& packet) = 0;
	virtual void OnWrite(RIOBuffer* buffer, int transferred) = 0;
	virtual void OnConnected() = 0;
	virtual void OnClose() = 0;

	virtual int PacketSize(std::istream& packet) = 0;

	std::atomic<SOCKET> rawSocket;
	SOCKADDR_IN addr;
	RIO_RQ requestQueue;
	std::mutex requestLock;
	std::vector<RIOBuffer*> readBuf;
	RIOServer* server;
};
}
