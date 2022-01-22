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

	static BufferDesc constant(size_t size_in_bytes, bool dynamic = true);
	static BufferDesc index(size_t size_in_bytes, bool dynamic = false);
	static BufferDesc vertex(size_t size_in_bytes, bool dynamic = false);
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
	
	static TextureDesc depth_stencil(DepthFormat format, UINT width, UINT height, UINT bind_flags, UINT mip_levels = 1, UINT sample_count = 1, UINT sample_quality = 0);
	
	// Set mip_levels = 0 to generate mip level textures all the way to the bottom!
	static TextureDesc make_2d(
		DXGI_FORMAT format,
		UINT width, 
		UINT height, 
		UINT bind_flags = D3D11_BIND_SHADER_RESOURCE,
		UINT mip_levels = 1, 
		UINT array_size = 1,
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
		UINT cpu_access_flags = 0,
		UINT sample_count = 1,
		UINT sample_quality = 0,
		UINT misc_flags = 0);

private:
	/*
		Remember that 1D and 3D descriptors are SUBSETS of the 2D descriptor! (check MSDN to verify)
		One caveat is to give ArraySize to Depth when filling 3D descriptor
		If you add 1D/3D, no need to change here, simply have 
			- convert_to_1d_desc()
			- convert_to_3d_desc()
		on branch time in create_texture!
	*/
	D3D11_TEXTURE2D_DESC m_desc{};
	TextureType m_type = TextureType::eNone;

	RenderTextureClear m_render_target_clear = RenderTextureClear::black();
	//DepthStencilClear m_depth_stencil_clear;
};

class InputLayoutDesc
{
	// https://gamedev.net/forums/topic/631296-what-is-the-point-of-multiple-vertex-buffers/4980091/

	friend class GfxDevice;
public:
	InputLayoutDesc() = default;
	InputLayoutDesc(const std::vector<D3D11_INPUT_ELEMENT_DESC>& descs) : m_input_descs(descs) {}
	InputLayoutDesc& append(LPCSTR semantic, DXGI_FORMAT format, UINT slot, InputClass input_type = InputClass::ePerVertex, UINT instanced_steprate = 0);
	InputLayoutDesc& append(const InputLayoutDesc& another_layout);


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

