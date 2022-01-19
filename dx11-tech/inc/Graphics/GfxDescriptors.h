#pragma once
#include "Graphics/GfxCommon.h"
#include "Graphics/GfxTypes.h"

class InputLayoutDesc
{
	friend class GfxApi;
public:
	InputLayoutDesc& add(D3D11_INPUT_ELEMENT_DESC desc);

private:
	std::vector<D3D11_INPUT_ELEMENT_DESC> m_descs;

};

class BufferDesc
{
	friend class GfxApi;
public:
	BufferDesc() = delete;
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
	TextureDesc() = delete;
	TextureDesc(const D3D11_TEXTURE2D_DESC& desc) : m_desc(desc), m_type(TextureType::e2D) {}

	static TextureDesc depth_stencil(DepthFormat format, UINT width, UINT height, UINT bind_flags, UINT mip_levels = 0);

private:
	D3D11_TEXTURE2D_DESC m_desc;
	TextureType m_type = TextureType::eNone;
};

class FramebufferDesc
{
public:
	FramebufferDesc& set_render_target(uint8_t slot, GPUTexture target);
	FramebufferDesc& set_depth_stencil(GPUTexture target);

private:
	std::array<std::optional<GPUTexture>, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_targets;
	std::optional<GPUTexture> m_depth_stencil;
};

class GraphicsPipelineDesc
{

};

class ComputePipelineDesc
{

};

class RenderPassDesc
{

};
