#include "stdafx.h"
#include "RIOSocket.h"
#include "RIOserver.h"
#include "RIOBuffer.h"
#include "RIOBufferPool.h"
#include "RIOStreamBuffer.h"
#include "RIOStatic.h"

namespace network
{

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
		Close(CLOSE_CALLBACK_STATUS_ERROR, std::string("error code ") + 
		std::to_string(status));
	}
	else if (transferred == 0)
	{
		Close(CLOSE_TRANSFERRED_SIZE_ZERO, "");
	}
	else
	{
		if (buf->IsReadRequest)
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

		int packet_length = Static::GetProtoPacketSize(packet);
		int curr_length = static_cast<int>(streamBuf.showmanyc());
		if (packet_length == 0 || packet_length < curr_length)
			break;

		if (packet_length < 0 || packet_length > PROTO_MAX_PACKET_LENGTH)
		{
			Close(CLOSE_INVALID_PACKET_LENGTH, "");
			return;
		}

		int left = currLength - packetLength;
		buf->SetSize(buf->GetSize() - left);

		OnRead(packet);
		
		std::copy(buf->GetRawBuf() + buffer->size,
				  buf->GetRawBuf() + buffer->size + left, buf->GetRawBuf());
		buf->SetSize(left);

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
			return false
	}

	return true;
}

bool RIOSocket::WriteBuf(const std::shared_ptr<RIOBuffer>& buf)
{
	buf->PrepareWrite();

	if (outstanding_write_ < Static::RIO_MAX_OUTSTANDING_WRITE)
	{
		if (!Static::RIOFunc()->RIOSend(rio_req_queue_, buf.get(), 1, NULL, buf.get()) &&
			WSAGetLastError() != WSA_IO_PENDING)
		{
			Static::PrintConsole(std::string("rio send fail error ") +
				std::to_string(WSAGetLastError()));
			return false;
		}

		++outstanding_write_;
		write_buf_.push_back(buf);
		self_container_.push_back(shared_from_this());
	}
	else
	{
		if (write_buf_.size() > Static::RIO_MAX_OUTSTANDING_WRITE &&
			write_buf_.back()->GetSize() + buf->GetSize() <= Static::RIO_BUFFER_SIZE)
		{
			std::copy(buf->GetRawBuf(), buf->GetRawBuf() + buf->GetSize(),
				write_buf_.back()->GetRawBuf());
			write_buf_.back()->SetSize(write_bufs.back()->GetSize() + buf->GetSize());
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
		Static::PrintConsole("write buf not match in write call back");
		throw std::logic_error("");
	}

	if (write_buf->GetSize() != transferred)
	{
		Static::PrintConsole("write buf transferred not matched");
		throw std::logic_error("");
	}

	if (write_buf_.size() <= outstanding_write_)
		return;

	auto iter = write_buf_.begin();
	std::advance(iter, outstanding_write_);

	(*iter)->PrepareWrite();

	if (!Static::RIOFunc()->RIOSend(rio_req_queue_, (*iter).get(), 1, NULL, (*iter).get() &&
		WSAGetLastError() != WSA_IO_PENDING))
	{
		Static::PrintConsole(std::string("rio send fail in write call back error ") +
			std::to_string(WSAGetLastError()));
		Close(CLOSE_SEND_FAIL_IN_WRITECALLBACK, "");
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
	readBuf.clear();
	readBuf.push_back(last);
}

void RIOSocket::Read()
{
	if (read_buf_.empty() || read_buf_.back()->IsFull())
		read_buf_.push_back(RIOBufferPool::GetInstance()->Alloc());

	read_buf_.back()->PrepareRead();

	std::lock_guard<std::mutex> lock(req_mutex_);
	if (!Static::RIOFunc()->RIOReceive(rio_req_queue_, read_buf_.back().get(),
		1, NULL, read_buf_.back().get()))
	{
		Static::PrintConsole(std::string("rio receive fail error ") +
			std::to_string(WSAGetLastError()));
		Close(CLOSE_READ_FAIL, "");
		return;
	}

	selfContainer.push_back(shared_from_this());
}

void RIOSocket::Close(loseReason reason, std::string str)
{
	auto before = rawSocket.exchange(INVALID_SOCKET);
	if (before != INVALID_SOCKET)
	{
		if (auto socketContainer = container.lock())
			socketContainer->DeleteSocket(shared_from_this());

		closesocket(before);
		OnClose(reason, str);
	}
}

SOCKET RIOSocket::GetRawSocket() const
{
	return rawSocket.load();
}

std::shared_ptr<RIOSocket> RIOSocket::PopFromSelfContainer()
{
	std::lock_guard<std::mutex> lock(requestLock);
	if (selfContainer.empty())
		return nullptr;

	auto pSocket = selfContainer.front();
	selfContainer.pop_front();

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
}