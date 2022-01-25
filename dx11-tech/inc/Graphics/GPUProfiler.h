#pragma once
#include "Graphics/GfxCommon.h"

// User can get a GPU Profiler from the GfxDevice
// Uses the internal annotator from DXDevice, not the user exposed GPUAnnotator
// References:
// https://www.reedbeta.com/blog/gpu-profiling-101/
// https://mynameismjp.wordpress.com/2011/10/13/profiling-in-dx11-with-queries/
class GPUProfiler
{
	friend class GfxDevice;
public:
	GPUProfiler(DXDevice* dev) : m_dev(dev) {}
	struct FrameData
	{
		std::map<std::string, std::pair<std::optional<D3D11_QUERY_DATA_PIPELINE_STATISTICS>, float>> profiles;
		float query_waiting_time = 0.f;
	};

	// add a scope to profile
	void begin(const std::string& name, bool annotate = true, bool get_pipeline_stats = true);
	void end(const std::string& name);


	// only available after frame ended
	const FrameData& get_frame_statistics();

private:
	GPUProfiler() = delete;
	GPUProfiler& operator=(const GPUProfiler&) = delete;
	GPUProfiler(const GPUProfiler&) = delete;

	// called internally by gfx dev (end frame is done before swapchain present, so theres room to visualize with the API)
	void frame_start();
	void frame_end();

private:
	DXDevice* m_dev;

	struct ProfileData
	{
		std::array<QueryPtr, gfxconstants::QUERY_LATENCY> disjoint;
		std::array<QueryPtr, gfxconstants::QUERY_LATENCY> timestamp_start;
		std::array<QueryPtr, gfxconstants::QUERY_LATENCY> timestamp_end;
		std::array<QueryPtr, gfxconstants::QUERY_LATENCY> pipeline_statistics;

		bool query_started = false;
		bool query_finished = false;
		bool annotate = false;
	};

	bool m_frame_started = false;
	bool m_frame_finished = false;

	std::array<FrameData, gfxconstants::QUERY_LATENCY> m_frame_datas{};
	std::map<std::string, ProfileData> m_profiles;
	uint64_t m_curr_frame = 0;

};


