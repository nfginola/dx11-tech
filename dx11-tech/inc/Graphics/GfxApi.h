#pragma once
#include "Graphics/DXDevice.h"
#include "Graphics/GfxCommon.h"

/*
	Once performance has been measured, only then should we allow binding multiple resources instead of single slot bindings.
	We need to explore GPU Queries before we make any optimizations.

*/
class GfxApi
{
public:
	// Book-keeping (e.g cleanup)
	void begin_frame();
	void end_frame();

	void create_buffer(const BufferDesc& desc, GPUBuffer* buffer);
	void create_texture_2d(const TextureDesc& desc, GPUTexture* texture);
	void create_sampler(const D3D11_SAMPLER_DESC& desc, Sampler* sampler);
	void create_shader(ShaderStage stage, const std::filesystem::path& fpath, Shader* shader);
	void create_input_layout(Shader shader, const InputLayoutDesc& desc, InputLayout* layout);
	void create_rasterizer_state(const D3D11_RASTERIZER_DESC1& desc, RasterizerState* rasterizer);
	void create_blend_state(const D3D11_BLEND_DESC1& desc, BlendState* rasterizer);
	void create_depth_stencil_state(const D3D11_DEPTH_STENCIL_DESC& desc, DepthStencilState* rasterizer);

	void begin_pass(const RenderPass& pass);	// Bind RTVs and pass
	void end_pass();							// Unbind RTVs and cleanup

	/*
		A draw is expected to be done between a begin_pass and end_pass!
		Otherwise, no Render Targets are set.
	*/
	
	void bind_vertex_buffer(UINT slot, const GPUBuffer* buffer, UINT stride, UINT offset);
	void bind_index_buffer(const GPUBuffer* buffer, DXGI_FORMAT format, UINT offset);

	void bind_constant_buffer(UINT slot, ShaderStage stage, const GPUBuffer* buffer);
	void bind_resource(UINT slot, ShaderStage stage, GPUAccess access, const GPUResource* resource);
	void bind_sampler(UINT slot, ShaderStage stage, const Sampler* sampler);
	void bind_pipeline(const GraphicsPipeline* pipeline);
	void bind_compute_pipeline(const ComputePipeline* pipeline);

	/*
	
	Need to experiment before use:
		RSSetScissorRects
	
	Priority implement:
		DrawIndexed
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
		DrawIndexedInstanced
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


