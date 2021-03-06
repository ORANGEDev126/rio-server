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
	RIOSocket(std::function<std::optional<uint32_t>(std::istream&)> length_func);
	virtual ~RIOSocket() = default;

	void Initialize(SOCKET sock, SOCKADDR_IN addr, RIO_RQ queue,
		const std::shared_ptr<RIOSocketContainer>& container);
	void OnIOCallBack(int status, int transferred, RIOBuffer* buf);
	void Read();
	bool Write(const std::vector<std::shared_ptr<RIOBuffer>>& bufs);
	void Close(std::string close_reason);
	SOCKET GetRawSocket() const;
	std::shared_ptr<RIOSocket> PopFromSelfContainer();

private:
	void OnReadCallBack(RIOBuffer* buf, int transferred);
	void OnWriteCallBack(RIOBuffer* buf, int transferred);
	void FreeReadBufUntilLast();
	bool WriteBuf(const std::shared_ptr<RIOBuffer>& buf);
	std::shared_ptr<RIOBuffer> PopFromWriteBuf(RIOBuffer* buf);
	std::string WSAErrorToString() const;

	virtual void OnRead(std::istream& packet) = 0;
	virtual void OnConnected() = 0;
	virtual void OnClose(std::string close_reason) = 0;
	virtual bool ValidPacketLength(int packet_length) = 0;

	SOCKET raw_sock_{ INVALID_SOCKET };
	SOCKADDR_IN addr_;
	std::atomic<RIO_RQ> rio_req_queue_{ RIO_INVALID_RQ };
	std::weak_ptr<RIOSocketContainer> container_;
	std::mutex req_mutex_;
	std::vector<std::shared_ptr<RIOBuffer>> read_buf_;
	std::list<std::shared_ptr<RIOBuffer>> write_buf_;
	int outstanding_write_{ 0 };
	std::list<std::shared_ptr<RIOSocket>> self_container_;
	std::function<std::optional<uint32_t>(std::istream&)> packet_length_func_;
};
}
