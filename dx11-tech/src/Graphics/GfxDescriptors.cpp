#include "pch.h"
#include "Graphics/GfxDescriptors.h"

InputLayoutDesc& InputLayoutDesc::add(D3D11_INPUT_ELEMENT_DESC desc)
{
	m_descs.push_back(desc);
	return *this;
}


BufferDesc BufferDesc::constant(UINT size, bool dynamic)
{
	size = size + (16 - (size % 16));	// 16 bytes align
	if (dynamic)
		return BufferDesc(CD3D11_BUFFER_DESC(size, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), BufferType::eConstant);
	else
		return BufferDesc(CD3D11_BUFFER_DESC(size, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DEFAULT), BufferType::eConstant);

}

BufferDesc BufferDesc::index(UINT size, bool dynamic)
{
	if (dynamic)
		return BufferDesc(CD3D11_BUFFER_DESC(size, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), BufferType::eIndex);
	else
		return BufferDesc(CD3D11_BUFFER_DESC(size, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_IMMUTABLE), BufferType::eIndex);
}

BufferDesc BufferDesc::vertex(UINT size, bool dynamic)
{
	if (dynamic)
		return BufferDesc(CD3D11_BUFFER_DESC(size, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), BufferType::eVertex);
	else
		return BufferDesc(CD3D11_BUFFER_DESC(size, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE), BufferType::eVertex);
}

TextureDesc TextureDesc::depth_stencil(DepthFormat format, UINT width, UINT height, UINT bind_flags, UINT mip_levels)
{
	// Depth stencil
	/*
		https://www.gamedev.net/forums/topic/691579-24bit-depthbuffer-is-a-sub-optimal-format/
		https://developer.nvidia.com/content/depth-precision-visualized

		32-bit depth + 8 bit stencil should be default.
		In create texture:
			- SRV should interpret the texture as DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS (32 bit depth readable)
			- DSV should interpret the texture as DXGI_FORMAT_D32_FLOAT_S8X24_UINT (32 bit depth + 8 bit stencil)
	*/

	if (format == DepthFormat::eD32)
		return TextureDesc(CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32_TYPELESS, width, height, 1, mip_levels, bind_flags));
	else if (format == DepthFormat::eD32_S8)
		return TextureDesc(CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32G8X24_TYPELESS, width, height, 1, mip_levels, bind_flags));
	else if (format == DepthFormat::eD24_S8)
		return TextureDesc(CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R24G8_TYPELESS, width, height, 1, mip_levels, bind_flags));
	else
	{
		assert(false);
		return CD3D11_TEXTURE2D_DESC{};
	}
}

FramebufferDesc& FramebufferDesc::set_render_target(uint8_t slot, GPUTexture target)
{
	assert(slot < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
	if (slot > 0)
		assert(m_targets[slot - 1].has_value());	// forbid gaps

	m_targets[slot] = target;
	return *this;
}

FramebufferDesc& FramebufferDesc::set_depth_stencil(GPUTexture target)
{
	m_depth_stencil = target;
	return *this;
}