#pragma once

namespace network { struct RIOBuffer; }

namespace network
{

class RIOStreamBuffer : public std::streambuf
{
public:
	RIOStreamBuffer(const std::vector<RIOBuffer*>& buffer);

	RIOStreamBuffer::int_type underflow() override;
	RIOStreamBuffer::int_type uflow() override;
	RIOStreamBuffer::int_type pbackfail(int ch = std::char_traits<char>::eof()) override;
	std::streamsize showmanyc() override;

private:
	std::vector<RIOBuffer*> buf;
	int currIndex;
	char* curr;
	char* begin;
	char* end;
};

}