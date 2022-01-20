#pragma once
#include "Graphics/GfxCommon.h"
#include "Graphics/GfxDescriptorsPrimitive.h"

// Strongly typed
template <typename T, typename Phantom>
class NamedType
{
public:
	explicit NamedType(T const& value) : m_value(value) {}
	T get() { return m_value; }
private:
	T m_value;
};


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
	bool is_valid() const { return m_internal_resource == nullptr; }

protected:
	GPUType() = default;		// Not a public object!

	DeviceChildPtr m_internal_resource;
};

class GPUResource : public GPUType
{
	friend class GfxApi;
protected:
	SrvPtr m_srv;
	UavPtr m_uav;
	RtvPtr m_rtv;
};

class GPUTexture : public GPUResource
{
	friend class GfxApi;
private:
	TextureType m_type = TextureType::eNone;
	TextureDesc m_desc;
	DsvPtr m_dsv;
};

class GPUBuffer : public GPUResource
{
	friend class GfxApi;
private:
	BufferType m_type = BufferType::eNone;
	BufferDesc m_desc;

};

class Shader : public GPUType
{
	friend class GraphicsPipeline;
	friend class GfxApi;
public:
	Shader() = default;
	operator Shader () { return *this; }
	ShaderStage get_stage() { return m_stage; }
private:
	ShaderStage m_stage = ShaderStage::eNone;
};

class Sampler : public GPUType { friend class GfxApi; };

class RasterizerState : public GPUType { friend class GfxApi; };

class InputLayout : public GPUType { friend class GfxApi; };

class BlendState : public GPUType { friend class GfxApi; };

class DepthStencilState : public GPUType { friend class GfxApi; };


using VertexShader = NamedType<Shader, struct VertexShaderPhantom>;
using PixelShader = NamedType<Shader, struct PixelShaderPhantom>;
using GeometryShader = NamedType<Shader, struct GeometryShaderPhantom>;
using HullShader = NamedType<Shader, struct HullShaderPhantom>;
using DomainShader = NamedType<Shader, struct DomainShaderPhantom>;
using ComputeShader = NamedType<Shader, struct ComputeShaderPhantom>;

class Framebuffer 
{
	friend class GfxApi;
public:
	Framebuffer() = default;
private:
	Framebuffer& operator=(const Framebuffer&) = delete;
	Framebuffer(const Framebuffer&) = default;
private:
	bool m_is_registered = false;

	std::array<GPUTexture, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_targets;
	GPUTexture m_depth_stencil_target;
};

class GraphicsPipeline 
{
	friend class GfxApi;
public:
	GraphicsPipeline() = default;
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

