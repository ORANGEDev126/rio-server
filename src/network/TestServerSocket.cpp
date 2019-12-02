#include "TestServerSocket.h"
#include "RIOStatic.h"
#include "RIOBufferPool.h"
#include "RIOBuffer.h"

namespace network
{

void TestServerSocket::OnConnected()
{
	Static::PrintConsole("test server socket connected");
}

void TestServerSocket::OnDisconnected(CloseReason reason, std::string str)
{
	Static::PrintConsole("test server socket disconnected");
}

void TestServerSocket::OnRead(std::istream& packet)
{
	int length = 0;
	if (!packet.read(reinterpret_cast<char*>(&length), 4))
		throw std::logic_error("read packet length fail");

	std::vector<char> v(std::istreambuf_iterator<char>(packet), {});

	if (v.size() != length - 4)
		throw std::logic_error("read packet length not same");

	std::vector<std::shared_ptr<RIOBuffer>> buf_group;
	auto buf = RIOBufferPool::GetInstance()->Alloc();
	buf_group.push_back(buf);

	*reinterpret_cast<int*>(buf->GetRawBuf()) = length;
	buf->SetSize(4);

	for (char c : v)
	{
		if (buf->IsFull())
			buf_group.push_back(RIOBufferPool::GetInstance()->Alloc());

		buf_group.back()->PutChar(c);
	}

	Write(buf_group);
}
}