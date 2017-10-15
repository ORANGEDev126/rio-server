#include <iostream>
#include <future>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <functional>
#include <map>
#include <unordered_map>
#include <atomic>

#include <Windows.h>

class ConstExprClass
{
public:
	constexpr ConstExprClass(int a, int b)
		: x(a)
		, y(b)
	{

	}

	constexpr int GetX() const { return x; }
	constexpr int GetY() const { return y; }

private:
	int x{ 0 };
	int y{ 0 };
};

class TestClass
{
public:
	TestClass()
	{

	}

	TestClass(const TestClass & rhs)
	{
		std::cout << "copy construct called" << std::endl;
	}

	TestClass & operator=(const TestClass & rhs)
	{
		std::cout << "copy assignment called" << std::endl;
	}

private:
	int x;
	int y;
	std::mutex m;
};

constexpr int GetInt(const ConstExprClass & rhs)
{
	if (rhs.GetX() == 10)
		return 5;
	else if (rhs.GetY() == 20)
		return 100;

	return rhs.GetX();
}

int main()
{
	TestClass t;
	TestClass t2(t);
	TestClass t3(std::move(t));

	t = t2;

	constexpr ConstExprClass c(10, 20);

	std::cout << "hoho" << std::endl;

	int a[GetInt(c)];


	return 0;
}