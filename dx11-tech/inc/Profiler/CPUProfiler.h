#pragma once
#include "Timer.h"

class CPUProfiler
{
public:
	static void initialize();
	static void shutdown();



	struct FrameData
	{
		std::map<std::string, float> profiles;
	};

	void begin(const std::string& name);
	void end(const std::string& name);

	void begin_accum(const std::string& name);
	void end_accum(const std::string& name);

	const FrameData& get_frame_statistics();

	void frame_start();
	void frame_end();

private:
	CPUProfiler() = default;
	CPUProfiler& operator=(const CPUProfiler&) = delete;
	CPUProfiler(const CPUProfiler&) = default;

private:
	bool m_frame_started = false;
	bool m_frame_finished = false;

	FrameData m_frame_data;
	std::map<std::string, std::pair<Timer, float>> m_profiles;		// { Timer, elapsed }
};

