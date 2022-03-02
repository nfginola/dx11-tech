#pragma once
#include "Graphics/API/GfxDescriptorsPrimitive.h"
#include "Graphics/API/GfxHandles.h"

/*	
	These descriptors are not "typical" descriptors such as for the Primitives.
	These directly use the GPU types! (hence why they are placed in a separate file)
*/

class RenderPassDesc
{
	friend class GfxDevice;
public:
	RenderPassDesc(
		std::vector<TextureHandle> render_targets,
		TextureHandle depth_stencil_target = TextureHandle{ 0 },
		std::vector<TextureHandle> resolve_targets = {},
		TextureHandle depth_stencil_resolve = TextureHandle{ 0 })
		: 
		m_depth_stencil_target(depth_stencil_target), 
		m_resolve_targets(resolve_targets), m_depth_stencil_target_resolve(depth_stencil_resolve) 
	{
		// Always set clear texture to black
		for (const auto& target_in : render_targets)
		{
			m_targets.push_back({ target_in, RenderTextureClear::black() });
		}
	};

	RenderPassDesc() = default;
	RenderPassDesc& operator=(const RenderPassDesc&) = default;
	RenderPassDesc(const RenderPassDesc&) = default;

private:
	std::vector<std::tuple<TextureHandle, RenderTextureClear>> m_targets;
	std::vector<TextureHandle> m_resolve_targets;
	TextureHandle m_depth_stencil_target;
	TextureHandle m_depth_stencil_target_resolve;
};

class PipelineDesc
{
	friend class GfxDevice;

	// Strongly typed shaders for safer public interface
	// https://www.fluentcpp.com/2016/12/08/strong-types-for-strong-interfaces/
public:
	PipelineDesc& set_shaders(VertexShader vs, PixelShader ps, std::optional<GeometryShader> gs = {}, std::optional<HullShader> hs = {}, std::optional<DomainShader> ds = {});
	PipelineDesc& set_topology(D3D11_PRIMITIVE_TOPOLOGY topology);
	PipelineDesc& set_input_layout(const InputLayoutDesc& desc);
	PipelineDesc& set_rasterizer(const RasterizerDesc& desc);
	PipelineDesc& set_blend(const BlendDesc& desc);
	PipelineDesc& set_sample_mask(UINT mask);
	PipelineDesc& set_depth_stencil(const DepthStencilDesc& desc);

	PipelineDesc() = default;
	PipelineDesc& operator=(const PipelineDesc&) = default;
	PipelineDesc(const PipelineDesc&) = default;

private:
	ShaderHandle m_vs, m_ps;
	std::optional<ShaderHandle> m_gs, m_hs, m_ds;

	// IA
	InputLayoutDesc m_input_desc;
	D3D11_PRIMITIVE_TOPOLOGY m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		
	// RS
	RasterizerDesc m_rasterizer_desc{};

	// OM
	BlendDesc m_blend_desc{};
	UINT m_sample_mask = 0xffffffff;
	DepthStencilDesc m_depth_stencil_desc;
};

class ComputePipelineDesc
{
	friend class GfxDevice;

public:
	ComputePipelineDesc& set_shader(ComputeShader cs);

private:
	ShaderHandle m_cs;

};
