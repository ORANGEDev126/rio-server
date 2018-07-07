#include "stdafx.h"
#include "ReferenceObject.h"

void ReferenceObject::IncRef()
{
	Count.fetch_add(1);
}

void ReferenceObject::DecRef()
{
	if (Count.fetch_sub(1) - 1 == 0)
	{
		std::cout << "delete object" << std::endl;
		delete this;
	}
}
