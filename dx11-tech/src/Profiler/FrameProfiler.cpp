#include "pch.h"
#include "Profiler/FrameProfiler.h"
#include <numeric>
#include <execution>

namespace perf
{
	FrameProfiler* profiler = nullptr;
}

void FrameProfiler::initialize(CPUProfiler* cpu, GPUProfiler* gpu)
{
	if (!perf::profiler)
		perf::profiler = new FrameProfiler(cpu, gpu);
}

void FrameProfiler::shutdown()
{
	if (perf::profiler)
		delete perf::profiler;
}

FrameProfiler::FrameProfiler(CPUProfiler* cpu, GPUProfiler* gpu) : m_cpu(cpu), m_gpu(gpu)
{
}


void FrameProfiler::begin_scope(const std::string& name, uint64_t flags)
{
	m_cpu->begin(name);
	m_gpu->begin(name, flags & PROFILER_GPU_ANNOTATE, flags & PROFILER_GPU_GET_PIPELINE_STATS);
}

void FrameProfiler::end_scope(const std::string& name)
{
	m_cpu->end(name);
	m_gpu->end(name);
}

void FrameProfiler::begin_cpu_scope(const std::string& name)
{
	m_cpu->begin(name);
}

void FrameProfiler::end_cpu_scope(const std::string& name)
{
	m_cpu->end(name);
}

void FrameProfiler::begin_gpu_scope(const std::string& name, uint64_t flags)
{

	m_gpu->begin(name, flags & PROFILER_GPU_ANNOTATE, flags & PROFILER_GPU_GET_PIPELINE_STATS);
}

void FrameProfiler::end_gpu_scope(const std::string& name)
{
	m_gpu->end(name);
}

void FrameProfiler::begin_cpu_scope_accum(const std::string& name)
{
	m_cpu->begin_accum(name);
}

void FrameProfiler::end_cpu_scope_accum(const std::string& name)
{
	m_cpu->end_accum(name);
}

const FrameProfiler::FrameData& FrameProfiler::get_frame_statistics()
{
	// We are okay giving zeroed data for some time until they are filled
	//assert(m_frame_finished == true);
	return m_frame_data;
}

void FrameProfiler::frame_start()
{	
	// GPU externally started by GfxDevice
	m_frame_started = true;
	m_frame_finished = false;
}


void FrameProfiler::frame_end()
{
	m_cpu->frame_end();
	// GPU externally ended by GfxDevice

	auto& cpu_frame_stats = m_cpu->get_frame_statistics();
	auto& gpu_frame_stats = m_gpu->get_frame_statistics();

	// CPU started immediately to capture the computations inbetween frame_end() and frame_start()
	m_cpu->frame_start();

	// Fill CPU and GPU times
	gather_data(cpu_frame_stats, gpu_frame_stats);

	// Note that calculating the averages takes quite some time!
	m_cpu->begin("Averaging Overhead");
	calculate_averages();
	m_cpu->end("Averaging Overhead");

	// Add full frame profile 
	for (const auto& averages : m_avg_gpu_times.profiles)
	{
		const auto& name = averages.first;
		const auto& pipeline_stats = averages.second;	
		const auto& avg_time = averages.second;
		m_frame_data.profiles[name].avg_gpu_time = avg_time;
	}

	for (const auto& averages : m_avg_cpu_times.profiles)
	{
		const auto& name = averages.first;
		const auto& avg_time = averages.second;
		m_frame_data.profiles[name].avg_cpu_time = avg_time;
	}

	m_frame_started = false;
	m_frame_finished = true;
	++m_curr_frame;
}

void FrameProfiler::gather_data(const CPUProfiler::FrameData& cpu_frame_stats, const GPUProfiler::FrameData& gpu_frame_stats)
{
	for (const auto& profile : cpu_frame_stats.profiles)
	{
		const auto& name = profile.first;
		const auto& time = profile.second;

		auto it = m_cpu_times.find(name);
		if (it == m_cpu_times.end())
			m_cpu_times.insert({ name, { time } });		// First profile insertion
		else
			it->second[m_curr_frame % s_averaging_frames] = time;
	}

	for (const auto& profile : gpu_frame_stats.profiles)
	{
		const auto& name = profile.first;
		const auto& time = profile.second;

		auto it = m_gpu_times.find(name);
		if (it == m_gpu_times.end())
			m_gpu_times.insert({ name, { time } });		// First profile insertion
		else
			it->second[m_curr_frame % s_averaging_frames] = time;
	}

	// lazy init persistent data structure
	if (m_is_first)
	{
		m_avg_gpu_times = gpu_frame_stats;
		m_is_first = false;
	}
}

void FrameProfiler::calculate_averages()
{
	if (m_curr_frame > s_averaging_frames)
	{
		/*
			Note that std::execution::seq is much faster than par_unseq for our use case.
			Probably since we dont have that many elements and its the threading overhead we see!
		*/

		// calc average cpu times per profile
		for (const auto& profile : m_cpu_times)
		{
			const auto& name = profile.first;
			const auto& times = profile.second;

			m_avg_cpu_times.profiles[name] = std::reduce(std::execution::seq, times.begin(), times.end()) / s_averaging_frames;
		}

		// calc average gpu times per profile
		for (const auto& profile : m_gpu_times)
		{
			const auto& name = profile.first;
			const auto& times = profile.second;

			// average
			m_avg_gpu_times.profiles[name] = std::reduce(std::execution::seq, times.begin(), times.end()) / s_averaging_frames;
		}
	}
}

FrameProfiler::Scoped::Scoped(const std::string& name, uint64_t flags) :
	m_name(name)
{
	assert(perf::profiler != nullptr);
	perf::profiler->begin_scope(name, flags);
}

FrameProfiler::Scoped::~Scoped()
{
	assert(perf::profiler != nullptr);
	perf::profiler->end_scope(m_name);
}

FrameProfiler::ScopedCPU::ScopedCPU(const std::string& name) :
	m_name(name)
{
	assert(perf::profiler != nullptr);
	perf::profiler->begin_cpu_scope(name);
}

FrameProfiler::ScopedCPU::~ScopedCPU()
{
	assert(perf::profiler != nullptr);
	perf::profiler->end_cpu_scope(m_name);
}

FrameProfiler::ScopedGPU::ScopedGPU(const std::string& name, uint64_t flags) :
	m_name(name)
{
	assert(perf::profiler != nullptr);
	perf::profiler->begin_gpu_scope(name, flags);
}

FrameProfiler::ScopedGPU::~ScopedGPU()
{
	assert(perf::profiler != nullptr);
	perf::profiler->end_gpu_scope(m_name);
}

FrameProfiler::ScopedCPUAccum::ScopedCPUAccum(const std::string& name)
{
	perf::profiler->begin_cpu_scope_accum(name);
	m_name = name;
}

FrameProfiler::ScopedCPUAccum::~ScopedCPUAccum()
{
	perf::profiler->end_cpu_scope_accum(m_name);

}

