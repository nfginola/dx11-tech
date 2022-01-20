#pragma once
#include "Graphics/GfxTypes.h"

/*	
	These descriptors are not "typical" descriptors such as for the Primitives.
	These directly use the GPU types! (hence why they are placed in a separate file)
*/

class FramebufferDesc
{
	friend class GfxApi;
public:
	FramebufferDesc(std::array<std::optional<GPUTexture>, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> render_targets, std::optional<GPUTexture> depth_stencil_target = {}) :
		m_targets(render_targets), m_depth_stencil_target(depth_stencil_target) {};

	FramebufferDesc() = default;
	FramebufferDesc& operator=(const FramebufferDesc&) = default;
	FramebufferDesc(const FramebufferDesc&) = default;

private:
	std::array<std::optional<GPUTexture>, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_targets;
	std::optional<GPUTexture> m_depth_stencil_target;
};

class PipelineDesc
{
	friend class GfxApi;
public:
	// Strongly typed for safer public interface
	// https://www.fluentcpp.com/2016/12/08/strong-types-for-strong-interfaces/
	PipelineDesc& set_shaders(VertexShader vs, PixelShader ps, std::optional<GeometryShader> gs = {}, std::optional<HullShader> hs = {}, std::optional<DomainShader> ds = {});

	PipelineDesc& set_topology(D3D11_PRIMITIVE_TOPOLOGY topology);
	PipelineDesc& set_input_layout(const InputLayoutDesc& desc);
	PipelineDesc& set_rasterizer(const RasterizerDesc& desc);
	PipelineDesc& set_blend(const BlendDesc& desc);
	PipelineDesc& set_depth_stencil(const DepthStencilDesc& desc);

	PipelineDesc() = default;
	PipelineDesc& operator=(const PipelineDesc&) = default;
	PipelineDesc(const PipelineDesc&) = default;

private:
	Shader m_vs, m_ps;
	std::optional<Shader> m_gs, m_hs, m_ds;

	// IA
	InputLayoutDesc m_input_desc;
	D3D11_PRIMITIVE_TOPOLOGY m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		
	// RS
	RasterizerDesc m_rasterizer_desc{};

	// OM
	BlendDesc m_blend_desc{};
	DepthStencilDesc m_depth_stencil_desc{};
};

class ComputePipelineDesc
{

};
