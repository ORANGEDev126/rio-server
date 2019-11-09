#pragma once

namespace network
{
class Static
{
public:
	static constexpr int RIO_BUFFER_SIZE = 8192;
	static constexpr int RIO_BUFFER_GRANULARITY = 65536;
	static int RIO_MAX_OUTSTANDING_WRITE;
	static int RIO_MAX_OUTSTANDING_READ;
	static int RIO_MAX_COMPLETION_QUEUE_SIZE;

	static void RIOStartUp();
	static void PrintConsole(std::string str);
	static RIO_EXTENSION_FUNCTION_TABLE* RIOFunc() { return &rio_func_table_; }

private:
	static RIO_EXTENSION_FUNCTION_TABLE rio_func_table_;

};
}