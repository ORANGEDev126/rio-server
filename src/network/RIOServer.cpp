#include "stdafx.h"
#include "RIOServer.h"
#include "RIOSocket.h"

namespace network
{
RIOServer::RIOServer(int threadCount)
	: threadContainer(std::make_shared<RIOThreadContainer>(threadCount))
	, socketContainer(std::make_shared<RIOSocketContainer>())
	, stop(true)
{

}

RIOServer::~RIOServer()
{
	Stop();
}

void RIOServer::Run()
{
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_REGISTERED_IO);
	if (listenSocket == INVALID_SOCKET)
	{
		auto error = WSAGetLastError();
		PrintConsole(std::string("create listen socket error ") + std::to_string(error));
		return;
	}

	SOCKADDR_IN addr = { 0, };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(GetPort());
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenSocket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) == SOCKET_ERROR)
	{
		auto error = WSAGetLastError();
		PrintConsole(std::string("bind socket error ") + std::to_string(error));
		return;
	}

	threadContainer->StartThread();
	acceptThread = std::make_unique<std::thread>([this]()
	{
		AcceptLoop();
	});
}

void RIOServer::Stop()
{
	stop = true;
	closesocket(listenSocket);

	for (auto& socket : socketContainer->GetAll())
		socket->Close();

	acceptThread->join();
}

void RIOServer::AcceptLoop()
{
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		auto error = WSAGetLastError();
		PrintConsole(std::string("listen error ") + std::to_string(error));
		return;
	}

	stop = false;
	PrintConsole(std::string("start server port : ") + std::to_string(GetPort()));

	for (;;)
	{
		auto acceptedSock = accept(listenSocket, NULL, NULL);

		if (stop)
			return;

		if (acceptedSock == INVALID_SOCKET)
		{
			auto error = WSAGetLastError();
			PrintConsole(std::string("invalid accepted socket ") + std::to_string(error));
			continue;
		}

		SOCKADDR_IN addr = { 0 };
		int addrLen = sizeof(addr);
		getpeername(acceptedSock, reinterpret_cast<SOCKADDR*>(&addr), &addrLen);

		auto socket = sockAllocator(acceptedSock, addr);
		if (!socket)
			continue;
		
		auto rq = threadContainer->BindSocket(acceptedSock, socket);
		if (rq)
		{
			socketContainer->AddSocket(socket);
			socket->Initialize(rq, socketContainer);
		}
	}
}
}