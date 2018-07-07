#pragma once

#include "RIOSocket.h"

class HTTPSocket : public RIOSocket
{
public:
	HTTPSocket(SOCKET rawSocket, const SOCKADDR_IN& addr);

private:
	virtual void OnRead(RIOBuffer* buffer, int transferred) override;
	virtual void OnWrite(RIOBuffer* buffer, int transferred) override;
	virtual void OnConnected() override;
	virtual void OnClose() override;
};