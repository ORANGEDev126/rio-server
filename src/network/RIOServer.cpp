#include "stdafx.h"
#include "RIOServer.h"
#include "RIOSocket.h"
#include "RIOStatic.h"
#include "RIOSocketContainer.h"
#include "RIOThreadContainer.h"

namespace network
{
RIOServer::~RIOServer()
{
	Stop();
}

void RIOServer::Run(int port)
{
	port_ = port;

	listen_sock_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_REGISTERED_IO);
	if (listen_sock_ == INVALID_SOCKET)
	{
		auto error = WSAGetLastError();
		Static::PrintConsole(std::string("create listen socket error ") + std::to_string(error));
		return;
	}

	SOCKADDR_IN addr = { 0, };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listen_sock_, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) == SOCKET_ERROR)
	{
		auto error = WSAGetLastError();
		Static::PrintConsole(std::string("bind socket error ") + std::to_string(error));
		return;
	}

	accept_thread_ = std::thread([this]()
	{
		AcceptLoop();
	});
}

void RIOServer::Stop()
{
	stop_ = true;
	closesocket(listen_sock_);

	for (auto& socket : container_->GetAll())
		socket->Close(RIOSocket::CloseReason::CLOSE_SERVER_STOP, 0);

	accept_thread_.join();
}

void RIOServer::AcceptLoop()
{
	if (listen(listen_sock_, SOMAXCONN) == SOCKET_ERROR)
	{
		auto error = WSAGetLastError();
		Static::PrintConsole(std::string("listen error ") + std::to_string(error));
		return;
	}

	stop_ = false;
	Static::PrintConsole(std::string("start server port : ") + std::to_string(port_));

	for (;;)
	{
		auto accepted_sock = accept(listen_sock_, NULL, NULL);

		if (stop_)
			return;

		if (accepted_sock == INVALID_SOCKET)
		{
			auto error = WSAGetLastError();
			Static::PrintConsole(std::string("invalid accepted socket ") + std::to_string(error));
			continue;
		}

		SOCKADDR_IN addr = { 0 };
		int addrLen = sizeof(addr);
		getpeername(accepted_sock, reinterpret_cast<SOCKADDR*>(&addr), &addrLen);

		auto socket = sock_allocator_();
		if (!socket)
			continue;
		
		RIO_RQ rq = RIOThreadContainer::GetInstance()->BindSocket(socket);
		if (rq != RIO_INVALID_RQ)
		{
			container_->AddSocket(socket);
			socket->Initialize(accepted_sock, addr, rq, container_);
		}
	}
}
}