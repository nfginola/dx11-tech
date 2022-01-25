#include "pch.h"
#include "FrameProfiler.h"
#include <numeric>
#include <execution>

namespace perf
{
	FrameProfiler* profiler = nullptr;
}

void FrameProfiler::initialize(unique_ptr<CPUProfiler> cpu, GPUProfiler* gpu)
{
	if (!perf::profiler)
		perf::profiler = new FrameProfiler(std::move(cpu), gpu);
}

void FrameProfiler::shutdown()
{
	if (perf::profiler)
		delete perf::profiler;
}

void FrameProfiler::begin_scope(const std::string& name, bool annotate, bool get_pipeline_stats)
{
	m_cpu->begin(name);
	m_gpu->begin(name, annotate, get_pipeline_stats);
}

void FrameProfiler::end_scope(const std::string& name)
{
	m_cpu->end(name);
	m_gpu->end(name);
}

const FrameProfiler::FrameData& FrameProfiler::get_frame_statistics()
{
	assert(m_frame_finished == true);
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

	grab_data();

	// CPU started immediately to capture the computations inbetween frame_end() and frame_start()
	// Otherwise the time taken to calculate averages is not included! (And its substantial!)
	m_cpu->frame_start();

	// Note that calculating the averages takes quite some time!
	//Timer time;
	calculate_averages();
	//auto elapsed = time.elapsed();
	//std::cout << "avg calc elapsed: " << elapsed << " ms\n";

	// Add full frame profile
	for (const auto& averages : m_avg_gpu_times.profiles)
	{
		const auto& name = averages.first;
		
		// GPU
		const auto& pipeline_stats = averages.second.first;
		const auto& avg_time = averages.second.second;

		auto& profile = m_frame_data.profiles[name];
		profile.avg_gpu_time = avg_time;
		//profile.avg_cpu_time = 
	}
	
	print_frame_results();

	m_frame_started = false;
	m_frame_finished = true;
	++m_curr_frame;
}

void FrameProfiler::grab_data()
{
	// grab cpu frame times
	auto& cpu_frame_stats = m_cpu->get_frame_statistics();

	for (const auto& profile : cpu_frame_stats.profiles)
	{
		const auto& name = profile.first;
		const auto& time = profile.second;

		auto it = m_frame_times.find(name);
		if (it == m_frame_times.end())
			m_frame_times.insert({ name, { time } });
		else
			it->second[m_curr_frame % s_averaging_frames] = time;
	}
	// grab gpu frame times
	auto& gpu_frame_stats = m_gpu->get_frame_statistics();
	for (const auto& profile : gpu_frame_stats.profiles)
	{
		const auto& name = profile.first;
		const auto& time = profile.second.second;

		auto it = m_data_times.find(name);
		if (it == m_data_times.end())
			m_data_times.insert({ name, { time } });
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
		// calc average cpu times per profile
		for (const auto& profile : m_frame_times)
		{
			const auto& name = profile.first;
			const auto& times = profile.second;

			m_avg_cpu_times.profiles[name] = std::reduce(std::execution::par_unseq, times.begin(), times.end()) / s_averaging_frames;
		}

		// calc average gpu times per profile
		for (const auto& profile : m_data_times)
		{
			const auto& name = profile.first;
			const auto& times = profile.second;

			// average
			m_avg_gpu_times.profiles[name].second = std::reduce(std::execution::par_unseq, times.begin(), times.end()) / s_averaging_frames;
		}
	}
}

void FrameProfiler::print_frame_results()
{
	// dont print until we have accumulated averages
	if (m_curr_frame < s_averaging_frames)
		return;

	// display cpu frametime
	//std::cout << "======= " << "*** Main Loop Frametime" << " =======" << "\n";
	//std::cout << m_avg_cpu_frame_time << " ms" << "\n\n";

	// display cpu frametime
	//std::cout << " ////////////// CPU TIMES '\\\\\\\\\\\\\\\\\\\\\\\'\n";
	for (const auto& profile : m_avg_cpu_times.profiles)
	{
		std::cout << "======= CPU: " << profile.first << " =======" << "\n";
		std::cout << profile.second << " ms" << "\n";

		std::cout << "\n";
	}

	// display gpu frametime
	//std::cout << " ////////////// GPU TIMES '\\\\\\\\\\\\\\\'\n";
	for (const auto& profile : m_avg_gpu_times.profiles)
	{
		std::cout << "======= GPU: " << profile.first << " =======" << "\n";
		std::cout << profile.second.second << " ms" << "\n";

		if (profile.second.first.has_value())
		{
			std::cout << profile.second.first->IAVertices << " : Vertices\n";
			std::cout << profile.second.first->CInvocations << " : Primitives sent to rasterizer\n";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}


