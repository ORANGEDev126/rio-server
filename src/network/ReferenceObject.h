#pragma once

class ReferenceObject
{
public:
	virtual ~ReferenceObject() = default;

	void IncreaseRef();
	void DecreaseRef();

private:
	std::atomic<int> Count{ 0 };
};