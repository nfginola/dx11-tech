#pragma once
#include "Graphics/GfxCommon.h"
#include "Graphics/GfxHelperTypes.h"

class BufferDesc
{
	friend class GfxApi;
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
	D3D11_BUFFER_DESC m_desc;
	BufferType m_type = BufferType::eNone;
};

class TextureDesc
{
	// Will probably extend to use Variants to handle Texture1D and Texture3D down the line.

	friend class GfxApi;
public:
	TextureDesc() = default;
	TextureDesc(const D3D11_TEXTURE2D_DESC& desc) : m_desc(desc), m_type(TextureType::e2D), m_render_target_clear(RenderTextureClear::black()) {}

	static TextureDesc depth_stencil(DepthFormat format, UINT width, UINT height, UINT bind_flags, UINT mip_levels = 0);

private:
	D3D11_TEXTURE2D_DESC m_desc;
	TextureType m_type = TextureType::eNone;
	RenderTextureClear m_render_target_clear = RenderTextureClear::black();
};



