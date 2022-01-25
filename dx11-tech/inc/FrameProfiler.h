#pragma once
#include "Graphics/GfxTypes.h"
#include "CPUProfiler.h"

class FrameProfiler
{
public:
	struct Profile
	{
		float avg_gpu_time;
		float avg_cpu_time;
	};

	struct FrameData
	{
		std::map<std::string, Profile> profiles;
	};

	FrameProfiler() = delete;
	FrameProfiler(CPUProfiler* cpu, GPUProfiler* gpu) : m_cpu(cpu), m_gpu(gpu) {}

	FrameProfiler& operator=(const FrameProfiler&) = delete;
	FrameProfiler(const FrameProfiler&) = delete;

	void begin(const std::string& name, bool annotate = true, bool get_pipeline_stats = true);
	void end(const std::string& name);

	const FrameData& get_frame_statistics();

	void frame_start();
	void frame_end();

	// temp
	void print_frame_results();

private:
	void calculate_averages();
	void grab_data();

private:
	CPUProfiler* m_cpu;
	GPUProfiler* m_gpu;

	static constexpr UINT s_averaging_frames = 500;

	std::map<std::string, std::array<float, s_averaging_frames>> m_data_times;		// GPU Frame times for each profile
	GPUProfiler::FrameData m_avg_gpu_times;											// Lazy initialized structure for GPU times averaging per profile
	
	std::map<std::string, std::array<float, s_averaging_frames>> m_frame_times{};	// CPU times
	CPUProfiler::FrameData m_avg_cpu_times;

	bool m_is_first = true;															// Lazy initialization checker

	FrameData m_frame_data;

	bool m_frame_started;
	bool m_frame_finished;

	uint64_t m_curr_frame = 0;

	//Timer m_full_frame_timer;

};

