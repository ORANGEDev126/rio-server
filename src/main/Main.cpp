#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <future>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <functional>
#include <map>
#include <unordered_map>
#include <atomic>
#include <regex>
#include <type_traits>
#include <array>
#include <iomanip>
#include <WinSock2.h>
#include <MSWSock.h>
#include <string>
#include <sstream>
#include <optional>

#include <Windows.h>

#include <boost\thread\future.hpp>

RIO_EXTENSION_FUNCTION_TABLE g_RIO;


void WorkerThread(RIO_CQ cq)
{
	RIORESULT result[256];

	for (;;)
	{
		auto size = g_RIO.RIODequeueCompletion(cq, result, 256);

		for (int i = 0; i < size; i++)
		{
			auto status = result[i].Status;
			auto transferred = result[i].BytesTransferred;
			auto socketContext = result[i].SocketContext;
			auto requestContext = result[i].RequestContext;

			if (status)
			{
				std::cout << "dequeue completion error " << status << std::endl;
			}
		}
	}
}

void IOCPWorkerThread(HANDLE hPort)
{
	DWORD transferred = 0;
	ULONG_PTR completionKey = 0;
	OVERLAPPED * overlapped;

	for (;;)
	{
		bool result = GetQueuedCompletionStatus(hPort, &transferred, &completionKey, &overlapped, INFINITE);
		if (!result)
		{
			auto error = WSAGetLastError();
			std::cout << "get queued completion error " << error << std::endl;
			continue;
		}

		std::cout << "get completion transferred " << transferred << std::endl;
	}
}

int AddFunction(int a, int b)
{
	int c = 20;

	int d = a + b + c;

	
	return d;
}

int main()
{
	int num = 10;
	int num2 = 20;

	int num3 = AddFunction(num, num2);

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	HANDLE hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	std::thread IOCPThread([&hPort]()
	{
		IOCPWorkerThread(hPort);
	});

	SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12000);
	addr.sin_addr.s_addr = inet_addr("112.175.222.6");

	if (connect(sock, (const sockaddr*)&addr, sizeof(addr)))
	{
		auto error = WSAGetLastError;
		std::cout << "connect error " << error << std::endl;
	}

	int bufferLength = 0;
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char*)&bufferLength, sizeof(int));

	CreateIoCompletionPort((HANDLE)sock, hPort, NULL, 0);

	std::stringstream ss;
	int length = 32 * 1024 * 1024;
	int protocol = 1;

	ss.write((char*)&length, sizeof(int));
	ss.write((char*)&protocol, sizeof(int));

	WSABUF wsaBuf;
	wsaBuf.buf = (char*)ss.str().c_str();
	wsaBuf.len = 10;

	OVERLAPPED overlapped = { 0, };
	DWORD dwSent = 0;
	DWORD dwRecv = 0;
	DWORD dwFlag = 0;
	
	if (WSARecv(sock, &wsaBuf, 1, &dwRecv, &dwFlag, &overlapped, NULL))
	{
		auto error = WSAGetLastError();
		std::cout << "recv error " << error << std::endl;
	}

	for (;;)
	{
		if (WSASend(sock, &wsaBuf, 1, &dwSent, 0, &overlapped, NULL))
		{
			auto error = WSAGetLastError();
			std::cout << "send error " << error << std::endl;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	/*SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_REGISTERED_IO);
	GUID tableID = WSAID_MULTIPLE_RIO;
	DWORD dwBytes = 0;

	if (WSAIoctl(sock, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &tableID, sizeof(GUID), (void**)&g_RIO, sizeof(g_RIO), &dwBytes, 0, 0))
	{
		auto error = WSAGetLastError();
		std::cout << "wsaioctl error " << std::endl;
	}

	auto cq = g_RIO.RIOCreateCompletionQueue(256, 0);
	if (cq == RIO_INVALID_CQ)
	{
		auto error = WSAGetLastError();
		std::cout << "create cq error " << error << std::endl;
	}

	std::thread work([&cq]()
	{
		WorkerThread(cq);
	});

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12000);
	addr.sin_addr.s_addr = inet_addr("112.175.222.6");

	for (;;)
	{
		if (connect(sock, (const sockaddr*)&addr, sizeof(addr)))
		{
			auto error = WSAGetLastError();
			std::cout << "connect error " << error << std::endl;
			continue;
		}

		break;
	}

	auto rq = g_RIO.RIOCreateRequestQueue(sock, 1, 1, 1, 1, cq, cq, 0);
	if (rq == RIO_INVALID_RQ)
	{
		auto error = WSAGetLastError();
		std::cout << "create rq error " << error << std::endl;
	}

	char * buf = new char[64];
	*((int*)buf) = 32 * 1024;
	*((int*)(buf + 4)) = 1;

	RIO_BUFFERID bufferID = g_RIO.RIORegisterBuffer(buf, 64);
	RIO_BUF RIOBuf;
	RIOBuf.BufferId = bufferID;
	RIOBuf.Offset = 0;
	RIOBuf.Length = 10;

	DWORD flags = 0;

	for (;;)
	{
		if (!g_RIO.RIOSend(rq, &RIOBuf, 1, flags, 0))
		{
			auto error = WSAGetLastError();
			std::cout << "send error " << error << std::endl;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}*/

	return 0;
}