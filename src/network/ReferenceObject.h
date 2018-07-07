#pragma once

class ReferenceObject
{
public:
	void IncRef();
	void DecRef();

protected:
	~ReferenceObject() = default;

private:
	std::atomic<int> Count{ 0 };
};