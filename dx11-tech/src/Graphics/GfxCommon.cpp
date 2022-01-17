#include "pch.h"
#include "Graphics/GfxCommon.h"

Framebuffer& Framebuffer::set(uint8_t slot, GPUTexture target)
{
	assert(slot < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
	if (slot > 1)
		assert(m_targets[slot - 1].has_value());	// forbid gaps

	m_targets[slot] = target;
	return *this;
}

void Framebuffer::validate()
{
	assert(m_targets[0].has_value());

	m_is_validated = true;
}



InputLayoutDesc& InputLayoutDesc::add(D3D11_INPUT_ELEMENT_DESC desc)
{
	m_descs.push_back(desc);
	return *this;
}

GraphicsPipeline& GraphicsPipeline::set_shader(Shader shader)
{
	switch (shader.m_stage)
	{
	case ShaderStage::eVertex:
		m_vs = shader;
		break;
	case ShaderStage::ePixel:
		m_ps = shader;
		break;
	case ShaderStage::eGeometry:
		m_gs = shader;
		break;
	case ShaderStage::eHull:
		m_hs = shader;
		break;
	case ShaderStage::eDomain:
		m_ds = shader;
		break;
	case ShaderStage::eCompute:
		assert(false);
		break;
	default:
		assert(false);
		break;
	}
	return *this;
}

GraphicsPipeline& GraphicsPipeline::set_input_layout(InputLayout layout)
{
	m_input_layout = layout;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::set_topology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	m_topology = topology;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::set_rasterizer(RasterizerState rasterizer_state)
{
	m_raster = rasterizer_state;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::set_blend(BlendState blend_state)
{
	m_blend = blend_state;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::set_depth_stencil(DepthStencilState depth_stencil_state)
{
	m_depth_stencil = depth_stencil_state;
	return *this;
}

void GraphicsPipeline::validate()
{
	// Bare minimum, rest can have defaults
	if (!m_vs.is_valid() || !m_ps.is_valid() || !m_input_layout.is_valid())
		assert(false);

	is_validated = true;
}



RenderPass& RenderPass::set_framebuffer(Framebuffer framebuffer)
{
	m_framebuffer = framebuffer;
	return *this;
}

RenderPass& RenderPass::set_ds_clear(DepthStencilClear clear)
{
	m_ds_clear = clear;
	return *this;
}

RenderPass& RenderPass::set_clear_values(UINT slot, RenderTextureClear clear)
{
	assert(slot < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
	if (slot > 1)
		assert(m_texture_clears[slot - 1].has_value());
	
	m_texture_clears[slot] = clear;
	return *this;
}

void RenderPass::validate()
{
	assert(m_framebuffer.m_is_validated);
	for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		if (m_framebuffer.m_targets[i].has_value() && !m_texture_clears[i].has_value())
			assert(false);		// Target[i] requires a Texture Clear
	}

	m_is_validated = true;
}



BufferDesc BufferDesc::make_constant(UINT size)
{
	size = size + (16 - (size % 16));	// 16 bytes align
	return BufferDesc(CD3D11_BUFFER_DESC(size, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), BufferType::eConstant);;
}

BufferDesc BufferDesc::make_index_immutable(UINT size)
{
	return BufferDesc(CD3D11_BUFFER_DESC(size, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_IMMUTABLE), BufferType::eIndex);
}

BufferDesc BufferDesc::make_vertex_immutable(UINT size)
{
	return BufferDesc(CD3D11_BUFFER_DESC(size, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE), BufferType::eVertex);
}
