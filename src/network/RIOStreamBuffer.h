#pragma once

namespace network { struct RIOBuffer; }

namespace network
{
class RIOStreamBuffer : public std::streambuf
{
public:
	RIOStreamBuffer(const std::vector<std::shared_ptr<RIOBuffer>>& buf);

	int_type underflow() override;
	int_type uflow() override;
	int_type pbackfail(int ch = std::char_traits<char>::eof()) override;
	std::streamsize showmanyc() override;

	void Reset();

private:
	std::vector<std::shared_ptr<RIOBuffer>> buf_;
	int curr_index_{ 0 };
	char* curr_{ nullptr };
	char* begin_{ nullptr };
	char* end_{ nullptr };
};
}