#include "pch.h"
#include "Graphics/GfxDescriptorsAbstraction.h"

PipelineDesc& PipelineDesc::set_shaders(VertexShader vs, PixelShader ps, std::optional<GeometryShader> gs, std::optional<HullShader> hs, std::optional<DomainShader> ds)
{
	// Catch errrors early for easy debugging
	assert(vs.get().get_stage() == ShaderStage::eVertex);
	assert(ps.get().get_stage() == ShaderStage::ePixel);

	m_vs = vs.get();
	m_ps = ps.get();
	m_gs = {};
	m_hs = {};
	m_ds = {};
	
	if (gs.has_value())
	{
		assert(gs->get().get_stage() == ShaderStage::eGeometry);
		m_gs = gs->get();
	}

	if (hs.has_value())
	{
		assert(hs->get().get_stage() == ShaderStage::eHull);
		m_hs = hs->get();
	}

	if (ds.has_value())
	{
		assert(ds->get().get_stage() == ShaderStage::eDomain);
		m_hs = ds->get();
	}

	return *this;
}

PipelineDesc& PipelineDesc::set_topology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	m_topology = topology;
	return *this;
}

PipelineDesc& PipelineDesc::set_input_layout(const InputLayoutDesc& desc)
{
	m_input_desc = desc;
	return *this;
}

PipelineDesc& PipelineDesc::set_rasterizer(const RasterizerDesc& desc)
{
	m_rasterizer_desc = desc;
	return *this;
}

PipelineDesc& PipelineDesc::set_blend(const BlendDesc& desc)
{
	m_blend_desc = desc;
	return *this;
}

PipelineDesc& PipelineDesc::set_sample_mask(UINT mask)
{
	m_sample_mask = mask;
	return *this;
}

PipelineDesc& PipelineDesc::set_depth_stencil(const DepthStencilDesc& desc)
{
	m_depth_stencil_desc = desc;
	return *this;
}

