#pragma once

#include "RIOSocket.h"

namespace network
{
class TestServerSocket : public RIOSocket
{
private:
	virtual void OnConnected() override;
	virtual void OnDisconnected(CloseReason reason, std::string str) override;
	virtual void OnRead(std::istream& packet) override;
};
}
