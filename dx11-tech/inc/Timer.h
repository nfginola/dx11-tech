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
	float elapsed(Unit unit = Unit::Milliseconds) const;

	void restart();

private:
	std::chrono::steady_clock::time_point m_start;

};

