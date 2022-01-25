#pragma once
#include "Graphics/GfxCommon.h"

// User can get an annotator from the GfxDevice
class GPUAnnotator
{
public:
	GPUAnnotator(AnnotationPtr annotator) : m_annotation(annotator) {}

	void begin_event(const std::string& name);
	void end_event();
	void set_marker(const std::string& name);

private:
	GPUAnnotator() = delete;
	GPUAnnotator& operator=(const GPUAnnotator&) = delete;
	GPUAnnotator(const GPUAnnotator&) = default;

private:
	AnnotationPtr m_annotation;
};

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


class RenderTextureClear
{
	friend class GfxDevice;
public:
	RenderTextureClear(std::array<float, 4> rgba = { 0.f, 0.f, 0.f, 1.f }) :
		m_rgba(rgba)
	{}

	static RenderTextureClear black() { return RenderTextureClear(); }

private:
	std::array<float, 4> m_rgba = { 0.f, 0.f, 0.f, 1.f };

};

class DepthStencilClear
{
	friend class GfxDevice;
public:
	DepthStencilClear(FLOAT depth, UINT8 stencil, UINT clear_flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL) :
		m_clear_flags(clear_flags),
		m_depth(depth),
		m_stencil(stencil)
	{}
	static DepthStencilClear d1_s0() { return DepthStencilClear(1.0f, 0); }

private:
	UINT m_clear_flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL;
	FLOAT m_depth = 1.0f;
	UINT8 m_stencil = 0;
};

class ReadWriteClear
{
	friend class GfxDevice;
public:
	ReadWriteClear() = delete;

	// Will trigger ClearUnorderedAccessViewFloat
	static ReadWriteClear fp(FLOAT a, FLOAT b, FLOAT c, FLOAT d) { return ReadWriteClear(std::array<FLOAT, 4>{a, b, c, d}); }

	// Will trigger ClearUnorderedAccessViewUint
	static ReadWriteClear uint(UINT a, UINT b, UINT c, UINT d) { return ReadWriteClear(std::array<UINT, 4>{a, b, c, d}); }

private:
	ReadWriteClear(std::variant<std::array<UINT, 4>, std::array<FLOAT, 4>> clear) : m_clear(clear) {};

private:
	std::variant<std::array<UINT, 4>, std::array<FLOAT, 4>> m_clear;
};

class SubresourceData
{
	friend class GfxDevice;
public:
	SubresourceData() = default;

	// https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_subresource_data
	// Pitch and Slice pitch only meaningful for 2D and 3D texture data respectively
	SubresourceData(void* data, UINT pitch = 0, UINT slice_pitch = 0) : m_subres({ data, pitch, slice_pitch }) {}
private:
	D3D11_SUBRESOURCE_DATA m_subres{ nullptr, 0, 0 };
};

struct ShaderBytecode
{
	shared_ptr<std::vector<uint8_t>> code;
	std::string fname;
	// filepath -- std::string -- either .hlsl or .cso
	// we can use the extension to check if the pipeline is reloadable
};



