#include "stdafx.h"
#include "RIOServer.h"
#include "RIOSocket.h"

RIOServer::RIOServer(int threadCount)
	: ThreadContainer(threadCount)
	, StopFlag(true)
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

	ThreadContainer.StartThread();
	AcceptThread = std::make_unique<std::thread>([this]()
	{
		AcceptLoop();
	});
}

void RIOServer::Stop()
{
	StopFlag = true;
	closesocket(listenSocket);

	for (auto& socket : SocketContainer.GetAll())
		socket->Close();

	AcceptThread->join();
}

void RIOServer::DeleteSocket(RIOSocket* sock)
{
	SocketContainer.DeleteSocket(sock);
}

void RIOServer::AcceptLoop()
{
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		auto error = WSAGetLastError();
		PrintConsole(std::string("listen error ") + std::to_string(error));
		return;
	}

	StopFlag = false;
	PrintConsole(std::string("start server port : ") + std::to_string(GetPort()));

	for (;;)
	{
		auto acceptedSock = accept(listenSocket, NULL, NULL);

		if (StopFlag)
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

		auto* socket = CreateSocket(acceptedSock, addr);
		if (socket)
		{
			auto rq = ThreadContainer.BindSocket(acceptedSock, socket);
			if (rq)
			{
				SocketContainer.AddSocket(socket);
				socket->Initialize(rq, this);
			}
		}
	}
}