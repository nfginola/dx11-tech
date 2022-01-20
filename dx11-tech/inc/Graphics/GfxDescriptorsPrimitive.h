#pragma once
#include "Graphics/GfxCommon.h"
#include "Graphics/GfxHelperTypes.h"

class BufferDesc
{
	friend class GfxDevice;
public:
	BufferDesc() = default;
	BufferDesc(const D3D11_BUFFER_DESC& desc) : m_desc(desc), m_type(BufferType::eCustom) {}
	BufferDesc(const D3D11_BUFFER_DESC& desc, BufferType type) : m_desc(desc), m_type(type) {}

	static BufferDesc constant(UINT size, bool dynamic = true);
	static BufferDesc index(UINT size, bool dynamic = false);
	static BufferDesc vertex(UINT size, bool dynamic = false);
	/*
		make others..
	*/

private:
	D3D11_BUFFER_DESC m_desc{};
	BufferType m_type = BufferType::eNone;
};

class TextureDesc
{
	// Will probably extend to use Variants to handle Texture1D and Texture3D down the line.

	friend class GfxDevice;
public:
	TextureDesc() = default;
	TextureDesc(const D3D11_TEXTURE2D_DESC& desc) : m_desc(desc), m_type(TextureType::e2D), m_render_target_clear(RenderTextureClear::black()) {}

	static TextureDesc depth_stencil(DepthFormat format, UINT width, UINT height, UINT bind_flags, UINT mip_levels = 0);

private:
	D3D11_TEXTURE2D_DESC m_desc{};
	TextureType m_type = TextureType::eNone;

	RenderTextureClear m_render_target_clear = RenderTextureClear::black();
	//DepthStencilClear m_depth_stencil_clear;
};

class InputLayoutDesc
{
	friend class GfxDevice;
public:
	InputLayoutDesc() = default;
	InputLayoutDesc(const std::vector<D3D11_INPUT_ELEMENT_DESC>& descs) : m_input_descs(descs) {}
	InputLayoutDesc& append(const D3D11_INPUT_ELEMENT_DESC& desc);

	// Assumes a VertexLayout type which has a static get_desc() function returning an std::vector<D3D11_INPUT_ELEMENT_DESC>
	template <typename VertexLayout>
	static InputLayoutDesc get_layout(UINT buffer_slot = 0, D3D11_INPUT_CLASSIFICATION input_slot_class = D3D11_INPUT_PER_VERTEX_DATA)
	{
		return VertexLayout::get_desc(buffer_slot, input_slot_class);
	}

private:
	std::vector<D3D11_INPUT_ELEMENT_DESC> m_input_descs;
};

class SamplerDesc
{
	friend class GfxDevice;
public:
	SamplerDesc() = default;
	SamplerDesc(const D3D11_SAMPLER_DESC& desc) : m_sampler_desc(desc) {}

private:
	/*
		Default: D3D11_FILTER_MIN_MAG_MIP_LINEAR, Tex Clamp, no comparison
		https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_sampler_desc
	*/
	D3D11_SAMPLER_DESC m_sampler_desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
};

class RasterizerDesc 
{
	friend class GfxDevice;
public:
	RasterizerDesc() = default;
	RasterizerDesc(const D3D11_RASTERIZER_DESC1& desc) : m_rasterizer_desc(desc) {}

private:
	/*
		Default: Solid, Cull Back, Front is Clockwise, Depth Enable
	*/
	D3D11_RASTERIZER_DESC1 m_rasterizer_desc = CD3D11_RASTERIZER_DESC1(CD3D11_DEFAULT());
};

class BlendDesc 
{
	friend class GfxDevice;
public:
	BlendDesc() = default;
	BlendDesc(const D3D11_BLEND_DESC1& desc) : m_blend_desc(desc) {}

private:
	/*
		Default:
			alpha2coverage off, no independent blend,
			1 * src + 0 * dst for color blend
			1 * src + 0 * dst for alpha blend
			no logic op
			color write all
	*/
	D3D11_BLEND_DESC1 m_blend_desc = CD3D11_BLEND_DESC1(CD3D11_DEFAULT());
};

class DepthStencilDesc 
{
	friend class GfxDevice;
public:
	DepthStencilDesc() = default;
	DepthStencilDesc(const D3D11_DEPTH_STENCIL_DESC& desc) : m_depth_stencil_desc(desc) {}

private:
	/*
		depth true, comparison less, depth write mask all, stencil off
	*/
	D3D11_DEPTH_STENCIL_DESC m_depth_stencil_desc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
};

