#include "stdafx.h"
#include "RIOStreamBuffer.h"
#include "RIOBuffer.h"
#include "RIOBufferPool.h"

namespace network
{
RIOStreamBuffer::RIOStreamBuffer(const std::vector<std::shared_ptr<RIOBuffer>>& buf)
	: buf_(buf)
{
	if (!buf.empty())
		Reset();
}

RIOStreamBuffer::int_type RIOStreamBuffer::underflow()
{
	if (curr_ == end_)
	{
		if(curr_index_ == buf_.size() - 1)
			return std::char_traits<char>::eof();

		++curr_index_;
		curr_ = buf_[curr_index_]->GetRawBuf();
		begin_ = curr_;
		end_ = begin_ + buf_[curr_index_]->GetSize();
	}

	return traits_type::to_int_type(*curr_);
}

RIOStreamBuffer::int_type RIOStreamBuffer::uflow()
{
	auto value = underflow();
	if (value != std::char_traits<char>::eof())
		++curr_;

	return value;
}

RIOStreamBuffer::int_type RIOStreamBuffer::pbackfail(int ch)
{
	if (curr_ == begin_)
	{
		if (curr_index_ == 0)
			return std::char_traits<char>::eof();

		--curr_index_;
		begin_ = buf_[curr_index_]->GetRawBuf();
		end_ = begin_ + buf_[curr_index_]->GetSize();
		curr_ = end_ - 1;
	}
	else
	{
		--curr_;
	}

	return traits_type::to_int_type(*curr_);
}

std::streamsize RIOStreamBuffer::showmanyc()
{
	int left = 0;
	for (int i = curr_index_ + 1; i < buf_.size(); ++i)
		left += buf_[i]->GetSize();

	return left + end_ - curr_;
}

void RIOStreamBuffer::Reset()
{
	curr_index_ = 0;
	curr_ = buf_[0]->GetRawBuf();
	begin_ = curr_;
	end_ = begin_ + buf_[0]->GetSize();
}
}