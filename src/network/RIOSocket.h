#pragma once

struct RIOBuffer;

class RIOSocket
{
public:
	RIOSocket();
	~RIOSocket();

	void OnIOCallBack(RIOBuffer* buffer, int transferred);


private:

};