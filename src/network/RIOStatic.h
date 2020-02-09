#pragma once

namespace network
{
class RIOStatic
{
public:
	static constexpr int RIO_BUFFER_SIZE = 8192;
	static constexpr int RIO_BUFFER_GRANULARITY = 65536;
	static constexpr int RIO_MAX_OUTSTANDING_WRITE = 16;
	static constexpr int RIO_MAX_OUTSTANDING_READ = 1;
	static constexpr int RIO_COMPLETION_QUEUE_BETA_FACTOR = 3;

	static void RIOStartUp();
	static void PrintConsole(std::string str);
	static RIO_EXTENSION_FUNCTION_TABLE* RIOFunc() { return &rio_func_table_; }
	static constexpr int CalculateRIOCompletionQueueSize(int max_conn)
	{
		return (RIO_MAX_OUTSTANDING_WRITE + RIO_MAX_OUTSTANDING_READ) *
			max_conn * RIO_COMPLETION_QUEUE_BETA_FACTOR;
	}

private:
	static RIO_EXTENSION_FUNCTION_TABLE rio_func_table_;

};
}