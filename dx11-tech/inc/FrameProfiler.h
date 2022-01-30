#pragma once
#include "Graphics/GPUProfiler.h"
#include "CPUProfiler.h"

enum ProfilerFlags
{
	PROFILER_GPU_ANNOTATE				= 1,
	PROFILER_GPU_GET_PIPELINE_STATS		= 2 << 0
};

/*
	Maybe add a stack based tracker for CPU and GPU respectively so we can keep track of scope nesting.
	Features for later.

	Use the profiler functions for fine grained scopes
	Use the Scope helpers for coarse-grained scopes where utilizing a scope { } doesn't harm readability
*/

class FrameProfiler
{
public:

public:
	// Scoped helpers
	class Scoped
	{
	public:
		Scoped() = delete;
		Scoped& operator=(const Scoped&) = delete;
		Scoped(const Scoped&) = delete;
	
		// Scoped CPU and GPU profiler
		Scoped(const std::string& name, uint64_t flags = PROFILER_GPU_ANNOTATE | PROFILER_GPU_GET_PIPELINE_STATS);
		~Scoped();

	private:
		std::string m_name;
	};

	class ScopedCPU
	{
	public:
		ScopedCPU() = delete;
		ScopedCPU& operator=(const ScopedCPU&) = delete;
		ScopedCPU(const ScopedCPU&) = delete;

		ScopedCPU(const std::string& name);
		~ScopedCPU();

	private:
		std::string m_name;
	};

	class ScopedGPU
	{
	public:
		ScopedGPU() = delete;
		ScopedGPU& operator=(const ScopedGPU&) = delete;
		ScopedGPU(const ScopedGPU&) = delete;

		ScopedGPU(const std::string& name, uint64_t flags = PROFILER_GPU_ANNOTATE | PROFILER_GPU_GET_PIPELINE_STATS);
		~ScopedGPU();

	private:
		std::string m_name;
	};

public:
	static void initialize(unique_ptr<CPUProfiler> cpu, GPUProfiler* gpu);
	static void shutdown();

	FrameProfiler() = delete;
	FrameProfiler(unique_ptr<CPUProfiler> cpu, GPUProfiler* gpu) : m_cpu(std::move(cpu)), m_gpu(gpu) {}

	FrameProfiler& operator=(const FrameProfiler&) = delete;
	FrameProfiler(const FrameProfiler&) = delete;

	struct Profile
	{
		float avg_gpu_time;
		float avg_cpu_time;

		// Pipeline statistics.. add sometime later
	};

	struct FrameData
	{
		std::map<std::string, Profile> profiles;
	};

	/*
		Tracks both CPU and GPU (default)
	*/
	void begin_scope(const std::string& name, uint64_t flags = PROFILER_GPU_ANNOTATE | PROFILER_GPU_GET_PIPELINE_STATS);
	void end_scope(const std::string& name);

	/*
		Independent CPU/GPU scopes if user would like to simply track one of them for a given scope
	*/
	void begin_cpu_scope(const std::string& name);
	void end_cpu_scope(const std::string& name);
	void begin_gpu_scope(const std::string& name, uint64_t flags = PROFILER_GPU_ANNOTATE | PROFILER_GPU_GET_PIPELINE_STATS);
	void end_gpu_scope(const std::string& name);

	const FrameData& get_frame_statistics();

	void frame_start();
	void frame_end();

private:
	void calculate_averages();
	void gather_data(const CPUProfiler::FrameData& cpu_frame_stats, const GPUProfiler::FrameData& gpu_frame_stats);

	// temp
	void print_frame_results();

private:
	// Maybe we want to apply some delta time to both below
	// Meaning we average over past X seconds and print every Y seconds?
	static constexpr UINT s_print_frame_freq = 170;		// Print every X frames
	static constexpr UINT s_averaging_frames = 350;		// Averaging over X frames

	unique_ptr<CPUProfiler> m_cpu;
	GPUProfiler* m_gpu;

	std::map<std::string, std::array<float, s_averaging_frames>> m_data_times;		// GPU Frame times for each profile
	GPUProfiler::FrameData m_avg_gpu_times;											// Lazy initialized structure for GPU times averaging per profile
	
	std::map<std::string, std::array<float, s_averaging_frames>> m_frame_times{};	// CPU times
	CPUProfiler::FrameData m_avg_cpu_times;

	bool m_is_first = true;															// Lazy initialization checker

	FrameData m_frame_data;

	bool m_frame_started = false;
	bool m_frame_finished = true;;

	uint64_t m_curr_frame = 0;


};


