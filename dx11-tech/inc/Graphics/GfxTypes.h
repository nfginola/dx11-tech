#pragma once
#include "Graphics/GfxCommon.h"
#include "Graphics/GfxDescriptorsPrimitive.h"

/*
	GPU related types/resources.
	We use an intermediary type so that:
		- We can keep extra state/helpers
		- Make the API "opaque" (meaning we dont directly touch the D3D11 inner workings)

	Also an educational experience to see how this kind of interface feels (pros/cons, workflow, etc.)
*/

class GPUType
{
public:
	bool is_empty() const { return m_internal_resource == nullptr; }

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
private:
	ShaderStage m_stage = ShaderStage::eNone;
};

class Sampler : public GPUType { friend class GfxApi; };

class RasterizerState : public GPUType { friend class GfxApi; };

class InputLayout : public GPUType { friend class GfxApi; };

class BlendState : public GPUType { friend class GfxApi; };

class DepthStencilState : public GPUType { friend class GfxApi; };

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

	std::array<std::optional<GPUTexture>, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_targets;
	std::optional<GPUTexture> m_depth_stencil;
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
	RasterizerState m_raster;

	// OM
	BlendState m_blend;
	DepthStencilState m_depth_stencil;
};

class ComputePipeline {};

class RenderPass 
{ 
	friend class GfxApi;
public:
	RenderPass() = default;
private:
	RenderPass& operator=(const RenderPass&) = delete;
	RenderPass(const RenderPass&) = default;

private:
	// Experiment first before we use it here
	//std::array<D3D11_BOX, MAX_SCISSORS> scissors;
	
	Framebuffer m_framebuffer;
	std::array<std::optional<D3D11_VIEWPORT>, GfxConstants::MAX_VIEWPORTS> m_viewports;
	std::optional<class DepthStencilClear> m_ds_clear;
};
