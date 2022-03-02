#include "pch.h"
#include "Profiler/CPUProfiler.h"

namespace perf
{
	CPUProfiler* cpu_profiler = nullptr;
}


void CPUProfiler::initialize()
{
	if (!perf::cpu_profiler)
		perf::cpu_profiler = new CPUProfiler();
}

void CPUProfiler::shutdown()
{
	if (perf::cpu_profiler)
		delete perf::cpu_profiler;
}

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

void CPUProfiler::begin_accum(const std::string& name)
{
	auto& p = m_profiles[name];
	auto& timer = p.first;
	timer.restart();
}

void CPUProfiler::end_accum(const std::string& name)
{
	auto& p = m_profiles[name];
	auto& timer = p.first;
	auto& elapsed = p.second;
	elapsed += timer.elapsed();		// Accumulative
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
	for (auto& profile : m_profiles)
	{
		const auto& name = profile.first;
		auto& time = profile.second.second;

		m_frame_data.profiles[name] = time;
		
		// reset
		time = 0.f;
	}
}

CPUProfiler::ScopedAccum::ScopedAccum(const std::string& name)
{
	perf::cpu_profiler->begin_accum(name);
	m_name = name;
}

CPUProfiler::ScopedAccum::~ScopedAccum()
{
	perf::cpu_profiler->end_accum(m_name);
}
