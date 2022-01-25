#include "pch.h"
#include "CPUProfiler.h"

void CPUProfiler::begin(const std::string& name)
{
	auto& p = m_profiles[name];
	auto& timer = p.first;
	timer.restart();
}

void CPUProfiler::end(const std::string& name)
{
	auto& p = m_profiles[name];
	auto& timer = p.first;
	auto& elapsed = p.second;
	elapsed = timer.elapsed();
}

const CPUProfiler::FrameData& CPUProfiler::get_frame_statistics()
{
	assert(m_frame_finished == true);
	return m_frame_data;
}

void CPUProfiler::frame_start()
{
	begin("*** Full Frame ***");
	m_frame_started = true;
	m_frame_finished = false;
}

void CPUProfiler::frame_end()
{
	end("*** Full Frame ***");
	m_frame_started = false;
	m_frame_finished = true;

	// Extract current frame data
	for (const auto& profile : m_profiles)
	{
		const auto& name = profile.first;
		const auto& time = profile.second.second;

		m_frame_data.profiles[name] = time;
	}
}
