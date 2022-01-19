#include "pch.h"
#include "Graphics/GfxDescriptorsAbstraction.h"

//FramebufferDesc& FramebufferDesc::set_render_target(uint8_t slot, GPUTexture target)
//{
//	assert(slot < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
//	if (slot > 0)
//		assert(m_targets[slot - 1].has_value());	// forbid gaps
//
//	m_targets[slot] = target;
//	return *this;
//}
//
//FramebufferDesc& FramebufferDesc::set_depth_stencil(GPUTexture target)
//{
//	m_depth_stencil = target;
//	return *this;
//}
