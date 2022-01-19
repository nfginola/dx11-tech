#pragma once
#include "Graphics/GfxTypes.h"

/*	
	These descriptors are not "typical" descriptors such as for the Primitives.
	These directly use the GPU types! (hence why they are separated into a separate file)
*/

class FramebufferDesc
{
	friend class GfxApi;
public:
	FramebufferDesc(std::array<std::optional<GPUTexture>, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> render_targets, std::optional<GPUTexture> depth_stencil = {}) :
		m_targets(render_targets), m_depth_stencil(depth_stencil) {};

	FramebufferDesc() = default;
	FramebufferDesc& operator=(const FramebufferDesc&) = default;
	FramebufferDesc(const FramebufferDesc&) = default;

private:
	std::array<std::optional<GPUTexture>, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_targets;
	std::optional<GPUTexture> m_depth_stencil;
};

// needs more work
class PipelineDesc
{
	friend class GfxApi;
public:
	PipelineDesc() = default;
	PipelineDesc& operator=(const PipelineDesc&) = default;
	PipelineDesc(const PipelineDesc&) = default;

private:
	Shader m_vs, m_ps, m_gs, m_hs, m_ds;

	// IA
	std::vector<D3D11_INPUT_ELEMENT_DESC> m_input_descs;
	D3D11_PRIMITIVE_TOPOLOGY m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		
	/*
		Descriptors are used the for resources below as re-creating them has no effect (existing ones are returned by D3D11 on creation)
	*/

	// RS
	D3D11_RASTERIZER_DESC1 m_rasterizer_desc;

	// OM
	D3D11_BLEND_DESC1 m_blend_desc;
	D3D11_DEPTH_STENCIL_DESC m_depth_stencil_desc;
};

class ComputePipelineDesc
{

};
