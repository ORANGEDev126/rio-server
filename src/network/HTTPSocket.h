#pragma once

#include "RIOSocket.h"

namespace network
{
class HTTPSocket : public RIOSocket
{
public:
	HTTPSocket(SOCKET rawSocket, const SOCKADDR_IN& addr);

private:
	virtual void OnRead(std::istream& buf) override;
	virtual void OnConnected() override;
	virtual void OnClose() override;
	virtual int PacketSize(std::istream& packet) override;
};
}