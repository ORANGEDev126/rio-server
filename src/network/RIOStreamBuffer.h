#pragma once

namespace network { struct RIOBuffer; }

namespace network
{

class RIOStreamBuffer : public std::streambuf
{
public:
	RIOStreamBuffer(const std::vector<RIOBuffer*>& buffer);

	int underflow() override;
	int uflow() override;
	int pbackfail(int ch = std::char_traits<char>::eof()) override;
	std::streamsize showmanyc() override;

private:
	std::vector<RIOBuffer*> buf;
	int currIndex;
	char* curr;
	char* begin;
	char* end;
};


}