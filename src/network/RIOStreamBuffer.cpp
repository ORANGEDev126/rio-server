#include "stdafx.h"
#include "RIOStreamBuffer.h"
#include "RIOBuffer.h"
#include "RIOBufferPool.h"

namespace network
{
RIOStreamBuffer::RIOStreamBuffer(const std::vector<RIOBuffer*>& buffer)
	: buf(buffer)
	, currIndex(0)
	, curr(nullptr)
	, begin(nullptr)
	, end(nullptr)
{
	if (!buf.empty())
	{
		begin = buf.front()->rawBuf;
		curr = begin;
		end = buf.back()->rawBuf + buf.back()->size;
	}
}

int RIOStreamBuffer::underflow()
{
	if (curr == end)
	{
		return std::char_traits<char>::eof();
	}
	else if (curr == buf[currIndex]->rawBuf + buf[currIndex]->size)
	{
		if (currIndex = buf.size() - 1)
			return std::char_traits<char>::eof();

		curr = buf[++currIndex]->rawBuf;
	}

	return *curr;
}

int RIOStreamBuffer::uflow()
{
	int value = underflow();
	if (value != std::char_traits<char>::eof())
		++curr;

	return value;
}

int RIOStreamBuffer::pbackfail(int ch)
{
	if (curr == begin || (ch != std::char_traits<char>::eof() && ch != curr[-1]))
		return std::char_traits<char>::eof();

	if (curr == buf[currIndex]->rawBuf)
	{
		--currIndex;
		curr = buf[currIndex]->rawBuf + buf[currIndex]->size - 1;
	}
	else
	{
		--curr;
	}

	return *curr;
}

std::streamsize RIOStreamBuffer::showmanyc()
{
	int left = 0;
	for (int i = currIndex + 1; i < buf.size(); ++i)
		left += buf[i]->size;

	return left + buf[currIndex]->rawBuf + buf[currIndex]->size - curr;
}
}