#include "stdafx.h"
#include "RIOStatic.h"
#include "RIOThreadContainer.h"

namespace network
{

int Static::RIO_MAX_OUTSTANDING_WRITE = 16;
int Static::RIO_MAX_OUTSTANDING_READ = 1;
int Static::RIO_MAX_COMPLETION_QUEUE_SIZE =
(RIO_MAX_OUTSTANDING_WRITE + RIO_MAX_OUTSTANDING_READ) * 50000;
RIO_EXTENSION_FUNCTION_TABLE Static::rio_func_table_{ 0, };

void Static::RIOStartUp()
{
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);

	SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_REGISTERED_IO);
	if (sock == INVALID_SOCKET)
	{
		PrintConsole("winsock start up make invalid socket");
		return;
	}

	GUID func_table_id = WSAID_MULTIPLE_RIO;
	DWORD bytes = 0;
	if (WSAIoctl(sock, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER,
		&func_table_id, sizeof(GUID),
		reinterpret_cast<void**>(&rio_func_table_), sizeof(rio_func_table_),
		&bytes, NULL, NULL))
	{
		auto error = WSAGetLastError();
		PrintConsole(std::string("get extension function table fail ") + std::to_string(error));
	}
}

void Static::PrintConsole(std::string str)
{
	static std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);
	std::cout << str << std::endl << std::flush;
}

uint32_t Static::GetProtoPacketSize(std::istream& stream)
{
	int length = 0;
	if (!stream.read(reinterpret_cast<char*>(&length), 4))
		return 0;

	return length;
}
}