#include "pch.h"
#include "Timer.h"

Timer::Timer()
{
	m_start = std::chrono::high_resolution_clock::now();
}

float Timer::elapsed(Unit unit) const
{
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float, std::milli> diff = end - m_start;	

	switch (unit)
	{
	case Unit::Seconds:
		return diff.count() / 1000.f;
	case Unit::Milliseconds:
		return diff.count();
	default:
		return diff.count();	// Default in seconds
	}
}
