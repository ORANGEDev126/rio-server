#include "stdafx.h"
#include "RIOSocket.h"
#include "RIOserver.h"
#include "RIOBuffer.h"
#include "RIOBufferPool.h"
#include "RIOStreamBuffer.h"
#include "RIOStatic.h"
#include "RIOSocketContainer.h"

namespace network
{

RIOSocket::RIOSocket(std::function<std::optional<uint32_t>(std::istream&)> length_func)
	: packet_length_func_(length_func)
{
}

void RIOSocket::Initialize(SOCKET sock, SOCKADDR_IN addr, RIO_RQ queue,
	const std::shared_ptr<RIOSocketContainer>& container)
{
	raw_sock_ = sock;
	addr_ = addr;
	rio_req_queue_ = queue;
	container_ = container;

	OnConnected();
	Read();
}

void RIOSocket::OnIOCallBack(int status, int transferred, RIOBuffer* buf)
{
	if (status)
	{
		Close("rio dequeue completion status error : " + std::to_string(status));
	}
	else if (transferred == 0)
	{
		Close("rio transferred length zero");
	}
	else
	{
		if (buf->IsReadRequest())
			OnReadCallBack(buf, transferred);
		else if(buf->IsWriteRequest())
			OnWriteCallBack(buf, transferred);
	}
}

void RIOSocket::OnReadCallBack(RIOBuffer* buf, int transferred)
{
	buf->SetSize(buf->GetSize() + transferred);

	for (;;)
	{
		RIOStreamBuffer stream_buf(read_buf_);
		std::istream packet(&stream_buf);

		auto packet_length = packet_length_func_(packet);
		auto curr_length = stream_buf.showmanyc();

		if (!packet_length)
			break;

		if (!ValidPacketLength(*packet_length))
			Close("invalid packet length");

		if (*packet_length < curr_length)
			break;

		auto left = curr_length - *packet_length;
		buf->SetSize(static_cast<int>(buf->GetSize() - left));

		OnRead(packet);
		
		std::copy(buf->GetRawBuf() + buf->GetSize(),
				  buf->GetRawBuf() + buf->GetSize() + left, buf->GetRawBuf());
		buf->SetSize(static_cast<int>(left));

		FreeReadBufUntilLast();
	}

	Read();
}

bool RIOSocket::Write(const std::vector<std::shared_ptr<RIOBuffer>>& bufs)
{
	std::unique_lock<std::mutex> lock(req_mutex_);
	for (const auto& buf : bufs)
	{
		if (!WriteBuf(buf))
			return false;
	}

	return true;
}

bool RIOSocket::WriteBuf(const std::shared_ptr<RIOBuffer>& buf)
{
	buf->PrepareWrite();

	if (outstanding_write_ < RIOStatic::RIO_MAX_OUTSTANDING_WRITE)
	{
		if (!RIOStatic::RIOFunc()->RIOSend(rio_req_queue_, buf.get(), 1, NULL, buf.get()) &&
			WSAGetLastError() != WSA_IO_PENDING)
		{
			RIOStatic::PrintConsole(std::string("rio send fail error ") + WSAErrorToString());
			return false;
		}

		++outstanding_write_;
		write_buf_.push_back(buf);
		self_container_.push_back(shared_from_this());
	}
	else
	{
		if (write_buf_.size() > RIOStatic::RIO_MAX_OUTSTANDING_WRITE &&
			write_buf_.back()->GetSize() + buf->GetSize() <= RIOStatic::RIO_BUFFER_SIZE)
		{
			std::copy(buf->GetRawBuf(), buf->GetRawBuf() + buf->GetSize(),
				write_buf_.back()->GetRawBuf());
			write_buf_.back()->SetSize(write_buf_.back()->GetSize() + buf->GetSize());
		}
		else
		{
			write_buf_.push_back(buf);
		}
	}

	return true;
}

void RIOSocket::OnWriteCallBack(RIOBuffer* buf, int transferred)
{
	std::unique_lock<std::mutex> lock(req_mutex_);

	--outstanding_write_;

	auto write_buf = PopFromWriteBuf(buf);
	if (!write_buf)
	{
		RIOStatic::PrintConsole("write buf not match in write call back");
		throw std::logic_error("");
	}

	if (write_buf->GetSize() != transferred)
	{
		RIOStatic::PrintConsole("write buf transferred not matched");
		throw std::logic_error("");
	}

	if (write_buf_.size() <= outstanding_write_)
		return;

	auto iter = write_buf_.begin();
	std::advance(iter, outstanding_write_);

	(*iter)->PrepareWrite();

	if (!RIOStatic::RIOFunc()->RIOSend(rio_req_queue_, (*iter).get(), 1, NULL, (*iter).get()) &&
		WSAGetLastError() != WSA_IO_PENDING)
	{
		RIOStatic::PrintConsole(std::string("rio send fail in write call back error ") +
			std::to_string(WSAGetLastError()));
		Close("rio send fail on write call back error code " + WSAErrorToString());
		return;
	}

	++outstanding_write_;
	self_container_.push_back(shared_from_this());
}

void RIOSocket::FreeReadBufUntilLast()
{
	if (read_buf_.empty() || read_buf_.size() == 1)
		return;

	auto last = read_buf_.back();
	read_buf_.clear();
	read_buf_.push_back(last);
}

void RIOSocket::Read()
{
	if (read_buf_.empty() || read_buf_.back()->IsFull())
		read_buf_.push_back(RIOBufferPool::GetInstance()->Alloc());

	read_buf_.back()->PrepareRead();

	std::lock_guard<std::mutex> lock(req_mutex_);
	if (!RIOStatic::RIOFunc()->RIOReceive(rio_req_queue_, read_buf_.back().get(),
		1, NULL, read_buf_.back().get()))
	{
		RIOStatic::PrintConsole(std::string("rio receive fail error ") +
			std::to_string(WSAGetLastError()));
		Close("rio receive fail error code " + WSAErrorToString());
		return;
	}

	self_container_.push_back(shared_from_this());
}

void RIOSocket::Close(std::string close_reason)
{
	auto before = rio_req_queue_.exchange(RIO_INVALID_RQ);
	if (before != RIO_INVALID_RQ)
	{
		if (auto container = container_.lock())
			container->DeleteSocket(shared_from_this());

		closesocket(raw_sock_);
		OnClose(close_reason);
	}
}

SOCKET RIOSocket::GetRawSocket() const
{
	return raw_sock_;
}

std::shared_ptr<RIOSocket> RIOSocket::PopFromSelfContainer()
{
	std::lock_guard<std::mutex> lock(req_mutex_);
	if (self_container_.empty())
		return nullptr;

	auto pSocket = self_container_.front();
	self_container_.pop_front();

	return pSocket;
}

std::shared_ptr<RIOBuffer> RIOSocket::PopFromWriteBuf(RIOBuffer* buf)
{
	int search_count = 0;

	for (auto iter = write_buf_.begin(); iter != write_buf_.end() ||
		search_count < outstanding_write_; ++iter, ++search_count)
	{
		auto write_buf = *iter;

		if (write_buf.get() == buf)
		{
			write_buf_.erase(iter);
			return write_buf;
		}
	}

	return nullptr;
}

std::string RIOSocket::WSAErrorToString() const
{
	return std::to_string(WSAGetLastError());
}
}