#pragma once
#include <optional>
#include <variant>
#include "Graphics/DXDevice.h"

static constexpr UINT MAX_SCISSORS = 4;
static constexpr UINT MAX_VIEWPORTS = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;

enum class ShaderStage { eNone, eVertex, eHull, eDomain, eGeometry, ePixel, eCompute};
enum class BufferType { eNone, eConstant, eVertex, eIndex, eStructured, eAppendConsume, eByteAddress, eRaw, eCustom };
enum class TextureType { eNone, e1D, e2D, e3D };
enum class GPUAccess { eRead, eReadWrite };
enum class DepthFormat { eD32, eD32_S8, eD24_S8 };

/*
	GPU related types/resources.
	We use an intermediary type so that:
		- We can keep extra state/helpers
		- Make the API "opaque" (meaning we dont directly touch the D3D11 inner workings)

	Also an educational experience to see how this kind of interface feels (pros/cons, workflow, etc.)

*/



/*
	Helpers
*/





//class Framebuffer
//{
//	friend class RenderPass;
//	friend class GfxApi;
//public:
//	// Set targets chronologically (0..15)
//	Framebuffer& validate();
//	
//	Framebuffer() = default;
//private:
//	Framebuffer& operator=(const Framebuffer&) = delete;
//	Framebuffer(const Framebuffer&) = default;
//
//private:
//	bool m_is_validated = false;
//
//	std::array<std::optional<GPUTexture>, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_targets;
//	std::optional<GPUTexture> m_depth_stencil;
//};
//
//class GraphicsPipeline
//{
//	friend class GfxApi;
//public:
//	GraphicsPipeline& set_shader(Shader shader);
//	GraphicsPipeline& set_input_layout(InputLayout layout);
//	GraphicsPipeline& set_topology(D3D11_PRIMITIVE_TOPOLOGY topology);
//	GraphicsPipeline& set_rasterizer(RasterizerState rasterizer_state);
//	GraphicsPipeline& set_blend(BlendState blend_state);
//	GraphicsPipeline& set_depth_stencil(DepthStencilState depth_stencil_state);
//	GraphicsPipeline& validate();
//
//	GraphicsPipeline() = default;
//private:
//	GraphicsPipeline& operator=(const GraphicsPipeline&) = default;
//	GraphicsPipeline(const GraphicsPipeline&) = default;
//
//private:
//	bool m_is_validated = false;
//
//	Shader m_vs, m_ps, m_gs, m_hs, m_ds;
//
//	// IA
//	InputLayout m_input_layout;
//	D3D11_PRIMITIVE_TOPOLOGY m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//
//	// RS
//	RasterizerState m_raster;
//
//	// OM
//	BlendState m_blend;
//	DepthStencilState m_depth_stencil;
//};
//
//class ComputePipeline
//{
//	// To be implemented
//};
//
//class RenderPass
//{
//	friend class GfxApi;
//public:
//	RenderPass& set_framebuffer(Framebuffer framebuffer);
//	RenderPass& set_ds_clear(DepthStencilClear clear);
//	RenderPass& set_clear_values(UINT slot, RenderTextureClear clear);
//	RenderPass& validate();
//
//	RenderPass() = default;
//private:
//	RenderPass& operator=(const RenderPass&) = default;
//	RenderPass(const RenderPass&) = default;
//
//private:
//	bool m_is_validated = false;
//
//	// Experiment first before we use it here
//	//std::array<D3D11_BOX, MAX_SCISSORS> scissors;
//	
//	Framebuffer m_framebuffer;
//	std::array<std::optional<D3D11_VIEWPORT>, MAX_VIEWPORTS> m_viewports;
//	std::array<std::optional<RenderTextureClear>, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_texture_clears;
//	std::optional<DepthStencilClear> m_ds_clear;
//};





