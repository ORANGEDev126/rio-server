#include "stdafx.h"
#include "network/RIOServer.h"
#include "network/RIOStatic.h"

int main()
{
	Static::WinsockStartUp();

	network::RIOServer s(std::thread::hardware_concurrency(), 80);
	s.Run();

	std::this_thread::sleep_for(std::chrono::hours(24));

	return 0;
}