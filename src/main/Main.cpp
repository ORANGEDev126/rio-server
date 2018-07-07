#include "stdafx.h"
#include "network/HTTPServer.h"

int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_REGISTERED_IO);
	if (sock == INVALID_SOCKET)
		std::cout << "invalid socket" << std::endl;
	
	GUID funcTableID = WSAID_MULTIPLE_RIO;
	DWORD dwBytes = 0;
	if (WSAIoctl(sock, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, 
		&funcTableID, sizeof(GUID), 
		reinterpret_cast<void**>(&g_RIO), sizeof(g_RIO), 
		&dwBytes, NULL, 
		NULL))
	{
		auto error = WSAGetLastError();
		std::cout << "get extension function table fail " << error << std::endl;
	}

	HTTPServer hs(std::thread::hardware_concurrency());
	hs.Run();

	std::this_thread::sleep_for(std::chrono::hours(24));

	return 0;
}