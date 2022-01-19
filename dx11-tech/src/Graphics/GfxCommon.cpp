#include "pch.h"
#include "Graphics/GfxCommon.h"







//Framebuffer& Framebuffer::validate()
//{
//	assert(m_targets[0].has_value() && "There must at least be one render target!");
//
//	m_is_validated.set(true);
//	return *this;
//}
//
//GraphicsPipeline& GraphicsPipeline::set_shader(Shader shader)
//{
//	m_is_validated.set(false);
//
//	switch (shader.m_stage)
//	{
//	case ShaderStage::eVertex:
//		m_vs = shader;
//		break;
//	case ShaderStage::ePixel:
//		m_ps = shader;
//		break;
//	case ShaderStage::eGeometry:
//		m_gs = shader;
//		break;
//	case ShaderStage::eHull:
//		m_hs = shader;
//		break;
//	case ShaderStage::eDomain:
//		m_ds = shader;
//		break;
//	case ShaderStage::eCompute:
//		assert(false);
//		break;
//	default:
//		assert(false);
//		break;
//	}
//	return *this;
//}
//
//GraphicsPipeline& GraphicsPipeline::set_input_layout(InputLayout layout)
//{
//	m_is_validated.set(false);
//
//	m_input_layout = layout;
//	return *this;
//}
//
//GraphicsPipeline& GraphicsPipeline::set_topology(D3D11_PRIMITIVE_TOPOLOGY topology)
//{
//	m_is_validated.set(false);
//
//	m_topology = topology;
//	return *this;
//}
//
//GraphicsPipeline& GraphicsPipeline::set_rasterizer(RasterizerState rasterizer_state)
//{
//	m_is_validated.set(false);
//
//	m_raster = rasterizer_state;
//	return *this;
//}
//
//GraphicsPipeline& GraphicsPipeline::set_blend(BlendState blend_state)
//{
//	m_is_validated.set(false);
//
//	m_blend = blend_state;
//	return *this;
//}
//
//GraphicsPipeline& GraphicsPipeline::set_depth_stencil(DepthStencilState depth_stencil_state)
//{
//	m_is_validated.set(false);
//
//	m_depth_stencil = depth_stencil_state;
//	return *this;
//}
//
//GraphicsPipeline& GraphicsPipeline::validate()
//{
//	// Bare minimum, rest can have defaults
//	if (m_vs.is_empty() || m_ps.is_empty() || m_input_layout.is_empty())
//		assert(false);
//
//	m_is_validated.set(true);
//	return *this;
//}
//
//
//
//RenderPass& RenderPass::set_framebuffer(Framebuffer framebuffer)
//{
//	m_is_validated.set(false);
//
//	m_framebuffer = framebuffer;
//	return *this;
//}
//
//RenderPass& RenderPass::set_ds_clear(DepthStencilClear clear)
//{
//	m_is_validated.set(false);
//
//	m_ds_clear = clear;
//	return *this;
//}
//
//RenderPass& RenderPass::set_clear_values(UINT slot, RenderTextureClear clear)
//{
//	m_is_validated.set(false);
//
//	assert(slot < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
//	if (slot > 0)
//		assert(m_texture_clears[slot - 1].has_value());
//	
//	m_texture_clears[slot] = clear;
//	return *this;
//}
//
//RenderPass& RenderPass::validate()
//{
//	assert(m_framebuffer.m_is_validated);
//	for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
//	{
//		if (m_framebuffer.m_targets[i].has_value() && !m_texture_clears[i].has_value())
//			assert(false);		// Target[i] requires a Texture Clear
//	}
//
//	m_is_validated.set(true);
//	return *this;
//}



