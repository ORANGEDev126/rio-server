#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#include <WinSock2.h>
#include <MSWSock.h>
#include <Windows.h>

#include <unordered_map>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <set>
#include <list>
#include <map>
#include <fstream>

#include <boost\thread\future.hpp>

extern RIO_EXTENSION_FUNCTION_TABLE g_RIO;

void PrintConsole(std::string str);
