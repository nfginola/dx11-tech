#pragma once
#include "Graphics/API/GfxCommon.h"
#include "Graphics/API/GfxDescriptorsPrimitive.h"
#include "ResourceHandleStack.h"
#include "Graphics/API/GfxHandles.h"


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



struct GPUType
{
	bool is_valid() const { return m_internal_resource != nullptr; }
	GPUType() = default;		// Not a public object!


	DeviceChildPtr m_internal_resource;

	// Resource handle
	res_handle handle = RES_INVALID_HANDLE;
	void free() { m_internal_resource.Reset(); }
};

struct GPUResource : public GPUType
{
	SrvPtr m_srv;
	UavPtr m_uav;
	RtvPtr m_rtv;
};

struct GPUTexture : public GPUResource
{
	friend class GfxDevice;
	friend class DiskTextureManager;

	TextureType m_type = TextureType::eNone;
	DsvPtr m_dsv;
};

struct GPUBuffer : public GPUResource
{

};

struct Shader : public GPUType
{
	Shader() = default;
	operator Shader () { return *this; }
	ShaderStage get_stage() { return m_stage; }

	bool operator==(const Shader& rhs) const { return m_internal_resource == rhs.m_internal_resource; };
	bool operator!=(const Shader& rhs) const { return !(m_internal_resource == rhs.m_internal_resource); };

	ShaderStage m_stage = ShaderStage::eNone;
	ShaderBytecode m_blob;
};

// Strongly typed shaders for safer public interface
// https://www.fluentcpp.com/2016/12/08/strong-types-for-strong-interfaces/
using VertexShader = NamedType<ShaderHandle, struct VertexShaderPhantom>;
using PixelShader = NamedType<ShaderHandle, struct PixelShaderPhantom>;
using GeometryShader = NamedType<ShaderHandle, struct GeometryShaderPhantom>;
using HullShader = NamedType<ShaderHandle, struct HullShaderPhantom>;
using DomainShader = NamedType<ShaderHandle, struct DomainShaderPhantom>;
using ComputeShader = NamedType<ShaderHandle, struct ComputeShaderPhantom>;

class Sampler : public GPUType 
{
	friend class GfxDevice;
	bool operator==(const Sampler& rhs) const { return m_internal_resource == rhs.m_internal_resource; };
	bool operator!=(const Sampler& rhs) const { return !(m_internal_resource == rhs.m_internal_resource); };
};

class RasterizerState : public GPUType 
{ 
	friend class GfxDevice; 
	bool operator==(const RasterizerState& rhs) const { return m_internal_resource == rhs.m_internal_resource; };
	bool operator!=(const RasterizerState& rhs) const { return !(m_internal_resource == rhs.m_internal_resource); };
};

class InputLayout : public GPUType 
{ 
	friend class GfxDevice; 
	bool operator==(const InputLayout& rhs) const { return m_internal_resource == rhs.m_internal_resource; };
	bool operator!=(const InputLayout& rhs) const { return !(m_internal_resource == rhs.m_internal_resource); };
};

class BlendState : public GPUType 
{ 
	friend class GfxDevice; 
	bool operator==(const BlendState& rhs) const { return m_internal_resource == rhs.m_internal_resource; };
	bool operator!=(const BlendState& rhs) const { return !(m_internal_resource == rhs.m_internal_resource); };
};

class DepthStencilState : public GPUType
{ 
	friend class GfxDevice;
	bool operator==(const DepthStencilState& rhs) const { return m_internal_resource == rhs.m_internal_resource; };
	bool operator!=(const DepthStencilState& rhs) const { return !(m_internal_resource == rhs.m_internal_resource); };
};




struct Framebuffer 
{
	Framebuffer() = default;
	Framebuffer& operator=(const Framebuffer&) = default;
	Framebuffer(const Framebuffer&) = default;

	bool m_is_registered = false;

	std::vector<std::tuple<TextureHandle, RenderTextureClear, DXGI_FORMAT, DXGI_SAMPLE_DESC>> m_targets;
	std::vector<TextureHandle> m_resolve_targets;
	//std::array<GPUTexture, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_targets;
	TextureHandle m_depth_stencil_target;
	TextureHandle m_depth_stencil_resolve_target;

	// Resource handle
	res_handle handle = RES_INVALID_HANDLE;
	void free() { m_is_registered = false; }
};

struct GraphicsPipeline 
{
	GraphicsPipeline() = default;
	~GraphicsPipeline() = default;
	GraphicsPipeline& operator=(const GraphicsPipeline&) = default;
	GraphicsPipeline(const GraphicsPipeline&) = default;

	bool m_is_registered = false;

	ShaderHandle m_vs, m_ps, m_gs, m_hs, m_ds;

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

	// Resource handle
	res_handle handle = RES_INVALID_HANDLE;
	void free() { m_is_registered = false; }
};

class ComputePipeline {};




