#pragma once

namespace network { struct RIOBuffer; }
namespace network { class RIOServer; }
namespace network { struct RIOBuffer; }
namespace network { class RIOSocketContainer; }

namespace network
{

class RIOSocket : public std::enable_shared_from_this<RIOSocket>
{
public:
	RIOSocket(SOCKET rawSock, const SOCKADDR_IN& addr);
	virtual ~RIOSocket();

	void Initialize(RIO_RQ queue, const std::shared_ptr<RIOSocketContainer>& socketContainer);
	void OnIOCallBack(int status, RIOBuffer* buffer, int transferred);
	void Read();
	bool Write(RIOBuffer* buffer);
	void Close();
	SOCKET GetRawSocket() const;
	std::shared_ptr<RIOSocket> PopFromSelfContainer();

private:
	void OnReadCallBack(RIOBuffer* buffer);
	void OnWriteCallBack(RIOBuffer* buffer);
	void FreeReadBufUntilLast();
	void FreeAllReadBuf();

	virtual void OnRead(std::istream& packet) = 0;
	virtual void OnConnected() = 0;
	virtual void OnClose() = 0;

	virtual int PacketSize(std::istream& packet) = 0;

	std::atomic<SOCKET> rawSocket;
	SOCKADDR_IN addr;
	RIO_RQ requestQueue;
	std::weak_ptr<RIOSocketContainer> container;
	std::mutex requestLock;
	std::vector<RIOBuffer*> readBuf;
	std::list<std::shared_ptr<RIOSocket>> selfContainer;
};

}
