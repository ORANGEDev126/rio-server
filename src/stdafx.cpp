#include "stdafx.h"

RIO_EXTENSION_FUNCTION_TABLE g_RIO = { 0, };

void PrintConsole(std::string str)
{
	static std::mutex m;
	static std::ofstream f("Log.txt", std::ios::out);
	std::lock_guard<std::mutex> lock(m);
	f << str << std::endl;
}