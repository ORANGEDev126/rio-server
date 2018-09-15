#include "stdafx.h"
#include "ReferenceObject.h"

namespace network
{
void ReferenceObject::IncRef()
{
	count.fetch_add(1);
}

void ReferenceObject::DecRef()
{
	if (count.fetch_sub(1) - 1 == 0)
	{
		PrintConsole("delete object");
		delete this;
	}
}
}
