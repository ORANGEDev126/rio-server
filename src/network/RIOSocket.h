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
	enum CloseReason
	{
		CLOSE_CALLBACK_STATUS_ERROR,
		CLOSE_TRANSFERRED_SIZE_ZERO,
		CLOSE_INVALID_PACKET_LENGTH,
		CLOSE_SEND_FAIL_IN_WRITECALLBACK,
		CLOSE_READ_FAIL
	};

	RIOSocket() = default;
	virtual ~RIOSocket() = default;

	void Initialize(SOCKET sock, SOCKADDR_IN addr, RIO_RQ queue,
		const std::shared_ptr<RIOSocketContainer>& container);
	void OnIOCallBack(int status, int transferred, RIOBuffer* buf);
	void Read();
	bool Write(const std::vector<std::shared_ptr<RIOBuffer>& bufs);
	void Close(CloseReason reason, std::string str);
	SOCKET GetRawSocket() const;
	std::shared_ptr<RIOSocket> PopFromSelfContainer();

private:
	void OnReadCallBack(RIOBuffer* buf, int transferred);
	void OnWriteCallBack(RIOBuffer* buf, int transferred);
	void FreeReadBufUntilLast();
	bool WriteBuf(const std::shared_ptr<RIOBuffer>& buf);
	std::shared_ptr<RIOBuffer> PopFromWriteBuf(RIOBuffer* buf);

	virtual void OnRead(std::istream& packet) = 0;
	virtual void OnConnected() = 0;
	virtual void OnClose(CloseReason reason, std::string str) = 0;

	std::atomic<SOCKET> raw_sock_{ INVALID_SOCKET; };
	SOCKADDR_IN addr_;
	RIO_RQ rio_req_queue_{ RIO_INVALID_RQ };
	std::weak_ptr<RIOSocketContainer> container_;
	std::mutex req_mutex_;
	std::vector<std::shared_ptr<RIOBuffer>> read_buf_;
	std::list<std::shared_ptr<RIOBuffer>> write_buf_;
	int outstanding_write_{ 0 };
	std::vector<std::shared_ptr<RIOSocket>> self_container_;
};
}
