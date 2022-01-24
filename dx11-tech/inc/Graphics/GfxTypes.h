#pragma once
#include "Graphics/GfxCommon.h"
#include "Graphics/GfxDescriptorsPrimitive.h"


/*
	GPU related types/resources.
	We use an intermediary type so that:
		- We can keep extra state/helpers
		- Make the API "opaque" (meaning we dont directly touch the D3D11 inner workings)

	Also an educational experience to see how this kind of interface feels (pros/cons, workflow, etc.)

	Rule of thumb im trying to follow:
		- Use std::optional for front-facing API (e.g Descriptors)
		- Use the GPUType "is_valid()" for internal checks
*/

class GPUType
{
public:
	bool is_valid() const { return m_internal_resource != nullptr; }

protected:
	GPUType() = default;		// Not a public object!

	DeviceChildPtr m_internal_resource;
};

class GPUResource : public GPUType
{
	friend class GfxDevice;
protected:
	SrvPtr m_srv;
	UavPtr m_uav;
	RtvPtr m_rtv;
};

class GPUTexture : public GPUResource
{
	friend class GfxDevice;
private:
	TextureDesc m_desc;
	DsvPtr m_dsv;
};

class GPUBuffer : public GPUResource
{
	friend class GfxDevice;
private:
	BufferDesc m_desc;
};

class Shader : public GPUType
{
	friend class GfxDevice;
public:
	Shader() = default;
	operator Shader () { return *this; }
	ShaderStage get_stage() { return m_stage; }
private:
	ShaderStage m_stage = ShaderStage::eNone;
	ShaderBytecode m_blob;
};

// Strongly typed shaders for safer public interface
// https://www.fluentcpp.com/2016/12/08/strong-types-for-strong-interfaces/
using VertexShader = NamedType<Shader, struct VertexShaderPhantom>;
using PixelShader = NamedType<Shader, struct PixelShaderPhantom>;
using GeometryShader = NamedType<Shader, struct GeometryShaderPhantom>;
using HullShader = NamedType<Shader, struct HullShaderPhantom>;
using DomainShader = NamedType<Shader, struct DomainShaderPhantom>;
using ComputeShader = NamedType<Shader, struct ComputeShaderPhantom>;

class Sampler : public GPUType { friend class GfxDevice; };

class RasterizerState : public GPUType { friend class GfxDevice; };

class InputLayout : public GPUType { friend class GfxDevice; };

class BlendState : public GPUType { friend class GfxDevice; };

class DepthStencilState : public GPUType { friend class GfxDevice; };




class Framebuffer 
{
	friend class GfxDevice;
public:
	Framebuffer() = default;
private:
	Framebuffer& operator=(const Framebuffer&) = delete;
	Framebuffer(const Framebuffer&) = default;
private:
	bool m_is_registered = false;
	
	std::vector<GPUTexture*> m_targets;
	std::vector<GPUTexture*> m_resolve_targets;
	//std::array<GPUTexture, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_targets;
	GPUTexture* m_depth_stencil_target = nullptr;
	GPUTexture* m_depth_stencil_resolve_target = nullptr;
};

class GraphicsPipeline 
{
	friend class GfxDevice;
public:
	GraphicsPipeline() = default;
	~GraphicsPipeline() = default;
private:
	GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
	GraphicsPipeline(const GraphicsPipeline&) = default;
private:
	bool m_is_registered = false;

	Shader m_vs, m_ps, m_gs, m_hs, m_ds;

	// IA
	InputLayout m_input_layout;
	D3D11_PRIMITIVE_TOPOLOGY m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// RS
	RasterizerState m_rasterizer;

	// OM
	BlendState m_blend;
	
	// Mimic PSO (add sample mask here)
	// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_graphics_pipeline_state_desc
	// https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-omsetblendstate (default sample mask value)
	UINT m_sample_mask = 0xffffffff;	

	DepthStencilState m_depth_stencil;
};

class ComputePipeline {};



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
	void begin_profile(const std::string& name, bool annotate = true, bool get_pipeline_stats = true);
	void end_profile(const std::string& name);


	// only available after frame ended
	const FrameData& get_frame_statistics();

private:
	GPUProfiler() = delete;
	GPUProfiler& operator=(const GPUProfiler&) = delete;
	GPUProfiler(const GPUProfiler&) = default;

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
