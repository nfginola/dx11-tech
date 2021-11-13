#pragma once
#include <chrono>

class Timer
{
public:
	enum class Unit
	{
		Seconds,
		Milliseconds
	};

	Timer();
	double elapsed(Unit unit = Unit::Seconds) const;

private:
	std::chrono::steady_clock::time_point m_start;

};

