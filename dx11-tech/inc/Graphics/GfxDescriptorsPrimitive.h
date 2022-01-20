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
	static InputLayoutDesc get_layout()
	{
		return VertexLayout::get_desc();
	}

private:
	std::vector<D3D11_INPUT_ELEMENT_DESC> m_input_descs;
};

/*

	To-do... (make helper creators just like above desc)

*/

class SamplerDesc
{

private:
	D3D11_SAMPLER_DESC m_sampler_desc{};
};

class RasterizerDesc 
{

private:
	D3D11_RASTERIZER_DESC1 m_rasterizer_desc{};
};

class BlendDesc 
{

private:
	D3D11_BLEND_DESC1 m_blend_desc{};
};

class DepthStencilDesc 
{

private:
	D3D11_DEPTH_STENCIL_DESC m_depth_stencil_desc{};
};

