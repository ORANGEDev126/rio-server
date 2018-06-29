#pragma once

#include "ReferenceObject.h"

struct RIOBuffer;

class RIOSocket : public ReferenceObject
{
public:
	RIOSocket();
	virtual ~RIOSocket();

	void OnIOCallBack(RIOBuffer* buffer, int transferred);
	void Read(RIOBuffer* buffer);
	void Write(RIOBuffer* buffer);
	SOCKET GetRawSocket() const;

private:
	virtual void OnRead(RIOBuffer* buffer);
	virtual void OnWrite(RIOBuffer* buffer);

	SOCKET RawSocket;
};