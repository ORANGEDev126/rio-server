#pragma once

namespace network { struct RIOBuffer; }
namespace network { class RIOServer; }
namespace network { struct RIOBuffer; }
namespace network { class RIOSocketContainer; }

namespace network
{

static constexpr int MAX_OUTSTANDING_SEND_SIZE = 64;

class RIOSocket : public std::enable_shared_from_this<RIOSocket>
{
public:
	RIOSocket(SOCKET rawSock, const SOCKADDR_IN& addr);
	virtual ~RIOSocket();

	void Initialize(RIO_RQ queue, const std::shared_ptr<RIOSocketContainer>& socketContainer);
	void OnIOCallBack(int status, const std::shared_ptr<RIOBuffer>& buffer, int transferred);
	void Read();
	bool Write(const std::shared_ptr<RIOBuffer>& buffer);
	void Close();
	SOCKET GetRawSocket() const;
	std::shared_ptr<RIOSocket> PopFromSelfContainer();

private:
	void OnReadCallBack(const std::shared_ptr<RIOBuffer> buffer);
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
