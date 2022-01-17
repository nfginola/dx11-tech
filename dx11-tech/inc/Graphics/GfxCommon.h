#pragma once
#include <optional>
#include <variant>
#include "Graphics/DXDevice.h"

constexpr UINT MAX_SCISSORS = 4;
constexpr UINT MAX_VIEWPORTS = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;

enum class ShaderStage { eNone, eVertex, eHull, eDomain, eGeometry, ePixel, eCompute};
enum class BufferType { eNone, eConstant, eVertex, eIndex, eStructured, eAppendConsume, eByteAddress, eRaw };
enum class TextureType { eNone, e1D, e2D, e3D };
enum class GPUAccess { eRead, eReadWrite };

/*
	GPU related types/resources.
	We use an intermediary type so that:
		- We can keep extra state/helpers
		- Make the API "opaque" (meaning we dont directly touch the D3D11 inner workings)

	Also an educational experience to see how this kind of interface feels (pros/cons, workflow, etc.)

*/
class GPUType
{
public:
	bool is_valid() const { return m_internal_resource != nullptr; }

protected:
	std::shared_ptr<void> m_internal_resource;
};

class GPUResource : public GPUType
{
	friend class GfxApi;
protected:
	std::shared_ptr<void> srv, uav, rtv;
};

class GPUTexture : public GPUResource 
{
	friend class GfxApi;
private:
	TextureType m_type = TextureType::eNone;
};

class GPUBuffer : public GPUResource 
{
	friend class GfxApi;
private:
	BufferType m_type = BufferType::eNone;
};

struct Shader : public GPUType 
{
	friend class GraphicsPipeline;
	friend class GfxApi;
private:
	ShaderStage m_stage = ShaderStage::eNone;
};

struct Sampler : public GPUType { friend class GfxApi; };

struct RasterizerState : public GPUType { friend class GfxApi; };

struct InputLayout : public GPUType { friend class GfxApi; };

struct BlendState : public GPUType { friend class GfxApi; };

struct DepthStencilState : public GPUType { friend class GfxApi; };


/*
	Helpers
*/
class RenderTextureClear
{
	friend class GfxApi;
public:
	RenderTextureClear(std::array<float, 4> rgba = { 0.f, 0.f, 0.f, 1.f }) :
		m_rgba(rgba)
	{}

	static RenderTextureClear black() { return RenderTextureClear(); }

private:
	std::array<float, 4> m_rgba = { 0.f, 0.f, 0.f, 1.f };

};

class DepthStencilClear
{
	friend class GfxApi;
public:
	DepthStencilClear(UINT clear_flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, FLOAT depth = 1.0f, UINT8 stencil = 0) :
		m_clear_flags(clear_flags),
		m_depth(depth),
		m_stencil(stencil)
	{}

	static DepthStencilClear depth_1f_stencil_0() { return DepthStencilClear(); }

private:
	UINT m_clear_flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL;
	FLOAT m_depth = 1.0f;
	UINT8 m_stencil = 0;
};

class ReadWriteClear
{
	friend class GfxApi;
public:
	ReadWriteClear() = delete;
	
	// Will trigger ClearUnorderedAccessViewFloat
	static ReadWriteClear fp(FLOAT a, FLOAT b, FLOAT c, FLOAT d) { return ReadWriteClear(std::array<FLOAT, 4>{a, b, c, d}); }

	// Will trigger ClearUnorderedAccessViewUint
	static ReadWriteClear uint(UINT a, UINT b, UINT c, UINT d) { return ReadWriteClear(std::array<UINT, 4>{a, b, c, d}); }

private:
	ReadWriteClear(std::variant<std::array<UINT, 4>, std::array<FLOAT, 4>> clear) : m_clear(clear) {};

private:
	std::variant<std::array<UINT, 4>, std::array<FLOAT, 4>> m_clear;
};


/*
	Abstractions for working with graphics

	Any and all higher level abstractions that make use of Graphics SHOULD hold on to these types.
*/ 

class Framebuffer
{
	friend class RenderPass;
	friend class GfxApi;
public:
	Framebuffer& set(uint8_t slot, GPUTexture target);
	void validate();

private:
	bool m_is_validated = false;

	std::array<std::optional<GPUTexture>, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_targets;
};

class GraphicsPipeline
{
	friend class GfxApi;
public:
	GraphicsPipeline& set_shader(Shader shader);
	GraphicsPipeline& set_input_layout(InputLayout layout);
	GraphicsPipeline& set_topology(D3D11_PRIMITIVE_TOPOLOGY topology);
	GraphicsPipeline& set_rasterizer(RasterizerState rasterizer_state);
	GraphicsPipeline& set_blend(BlendState blend_state);
	GraphicsPipeline& set_depth_stencil(DepthStencilState depth_stencil_state);
	void validate();

private:
	bool is_validated = false;

	Shader m_vs, m_ps, m_gs, m_hs, m_ds;

	// IA
	InputLayout m_input_layout;
	D3D11_PRIMITIVE_TOPOLOGY m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// RS
	RasterizerState m_raster;

	// OM
	BlendState m_blend;
	DepthStencilState m_depth_stencil;
};

class ComputePipeline
{
	// To be implemented
};

class RenderPass
{
	friend class GfxApi;
public:
	RenderPass& set_framebuffer(Framebuffer framebuffer);
	RenderPass& set_ds_clear(DepthStencilClear clear);
	RenderPass& set_clear_values(UINT slot, RenderTextureClear clear);
	void validate();

private:
	bool m_is_validated = false;

	// Experiment first before we use it here
	//std::array<D3D11_BOX, MAX_SCISSORS> scissors;
	
	Framebuffer m_framebuffer;
	std::array<std::optional<D3D11_VIEWPORT>, MAX_VIEWPORTS> m_viewports;
	std::array<std::optional<RenderTextureClear>, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_texture_clears;
	std::optional<DepthStencilClear> m_ds_clear;
};


/*
	Descriptors

	Using custom BufferDesc/TextureDesc in-case of future extensions (e.g specialized views)
*/
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
	BufferDesc(const D3D11_BUFFER_DESC& desc) : m_desc(desc) {};

private:
	D3D11_BUFFER_DESC m_desc;
};

class TextureDesc
{
	// Will probably extend to use Variants to handle Texture1D and Texture3D down the line.

	friend class GfxApi;
public:
	TextureDesc() = delete;
	TextureDesc(const D3D11_TEXTURE2D_DESC& desc) : m_desc(desc) {};

	D3D11_TEXTURE2D_DESC m_desc;
};





