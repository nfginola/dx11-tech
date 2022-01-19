#pragma once
#include "Graphics/GfxCommon.h"
#include "Graphics/GfxDescriptors.h"
#include "Graphics/GfxTypes.h"
#include "Graphics/GfxHelpers.h"

/*
	Once performance has been measured, only then should we allow binding multiple resources instead of single slot bindings.
	We need to explore GPU Queries before we make any optimizations.
	
	This API is D3D11 ONLY and is meant for:
		- Exploration (exposing the learning "surface area" in a controlled manner)
		- Techniques (using the new learned tools for practical purposes)

	API is "Lego Bricks" style!
		1. Start by creating GPU types and resources
		2. Assemble the appropriate type/resources into
			- Framebuffers
			- GraphicsPipeline
			- RenderPass

		3. Give the assembled bricks to the API
		4. Give GPUResources to the API (Binding Buffers/Textures/Samplers)
*/
class GfxApi
{
public:
	// Book-keeping (e.g cleanup)
	void begin_frame();					// Should clear backbuffer automatically
	void end_frame();

	// Create GPU primitives
	void create_buffer(const BufferDesc& desc, GPUBuffer* buffer, std::optional<SubresourceData> subres = {});
	void create_texture(const TextureDesc& desc, GPUTexture* texture, std::optional<SubresourceData> subres = {});
	void create_sampler(const D3D11_SAMPLER_DESC& desc, Sampler* sampler);
	void create_shader(ShaderStage stage, const std::filesystem::path& fpath, Shader* shader);
	void create_input_layout(Shader shader, const InputLayoutDesc& desc, InputLayout* layout);
	void create_rasterizer_state(const D3D11_RASTERIZER_DESC1& desc, RasterizerState* rasterizer);
	void create_blend_state(const D3D11_BLEND_DESC1& desc, BlendState* rasterizer);
	void create_depth_stencil_state(const D3D11_DEPTH_STENCIL_DESC& desc, DepthStencilState* rasterizer);

	void create_pipeline(GraphicsPipeline* pipeline);
	void create_compute_pipeline(ComputePipeline* pipeline);
	void create_renderpass(RenderPass* rp);

	void begin_pass(const RenderPass* pass);	// Bind RTVs and pass

	/*
		A draw is expected to be done between a begin_pass and end_pass!
		Otherwise, no Render Targets are set (D3D11 will complain)
	*/

	void end_pass();							// Unbind RTVs and cleanup

	
	// No need to check for type, D3D11 will do it for us
	void bind_vertex_buffer(UINT slot, const GPUBuffer* buffer, UINT stride, UINT offset);
	void bind_index_buffer(const GPUBuffer* buffer, DXGI_FORMAT format, UINT offset);
	void bind_constant_buffer(UINT slot, ShaderStage stage, const GPUBuffer* buffer);

	// Bind views, no need to known underlying type
	void bind_resource(UINT slot, ShaderStage stage, GPUAccess access, const GPUResource* resource);

	// Sampler :)
	void bind_sampler(UINT slot, ShaderStage stage, const Sampler* sampler);

	// Bind helpers
	void bind_pipeline(const GraphicsPipeline* pipeline);
	void bind_compute_pipeline(const ComputePipeline* pipeline);

	/*
	

	
	Need to experiment before use:
		RSSetScissorRects
	
	Priority implement:
		DrawIndexedInstanced
		Map
		Unmap
		UpdateSubresource
	
	Second prio:
		ResolveSubresource
		Begin
		End
		GetData
		SetPredication
		GetPredication
		Dispatch
		GenerateMips? (maybe automatic?)

	Back Burner:
		void clear_readwrite_resource(const GPUResource* resource, ReadWriteClear clear);
		void copy_resource(const GPUResource* dst, const GPUResource* src);
		ID3D11DeviceContext::CopySubresourceRegion

		GetResourceMinLOD??

		DispatchIndirect
		Draw
		DrawAuto
		DrawIndexed
		DrawIndexedInstancedIndirect
		DrawInstanced
		DrawInstancedIndirect



	*/

public:
	GfxApi() = delete;
	GfxApi(unique_ptr<class DXDevice> dev);
	~GfxApi();

	GfxApi& operator=(const GfxApi&) = delete;
	GfxApi(const GfxApi&) = delete;

private:


private:
	unique_ptr<DXDevice> m_dev;


};


