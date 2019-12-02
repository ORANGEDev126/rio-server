#include "TestClientManager.h"

namespace network
{
void TestClientManager::TestClientSocket::OnConnected()
{
}

void TestClientManager::TestClientSocket::OnDisconnected(
	CloseReason reason, std::string str)
{
}

void TestClientManager::TestClientSocket::OnRead(std::istream& packet)
{

}

void TestClientManager::StartTest(int conn_thread_count,
	int dis_thread_count, int send_thread_count)
{

}
}