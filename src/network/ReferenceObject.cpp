#include "stdafx.h"
#include "ReferenceObject.h"

void ReferenceObject::IncreaseRef()
{
	Count.fetch_add(1);
}

void ReferenceObject::DecreaseRef()
{
	if (Count.fetch_sub(1) - 1 == 0)
		delete this;
}
