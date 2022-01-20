#pragma once
#include "Graphics/GfxCommon.h"
#include "Graphics/GfxDescriptorsPrimitive.h"
#include "Graphics/GfxDescriptorsAbstraction.h"
#include "Graphics/GfxTypes.h"
#include "Graphics/GfxHelperTypes.h"

/*
	Once performance has been measured, only then should we allow binding multiple resources instead of single slot bindings.
	We need to explore GPU Queries before we make any optimizations.
	
	This API is D3D11 ONLY and is meant for:
		- Exploration (exposing the learning "surface area" in a controlled manner)
		- Techniques (using the new learned tools for practical purposes)

*/
class GfxDevice
{
public:
	// Book-keeping (e.g cleanup)
	void begin_frame();					// Should clear backbuffer automatically
	void end_frame();

	// Create GPU primitives
	void create_buffer(const BufferDesc& desc, GPUBuffer* buffer, std::optional<SubresourceData> subres = {});
	void create_texture(const TextureDesc& desc, GPUTexture* texture, std::optional<SubresourceData> subres = {});
	void create_sampler(const SamplerDesc& desc, Sampler* sampler);
	void create_shader(ShaderStage stage, const std::filesystem::path& fpath, Shader* shader);

	GPUTexture get_backbuffer();

	// Create GPU abstractions
	void create_framebuffer(const FramebufferDesc& desc, Framebuffer* framebuffer);
	void create_pipeline(const PipelineDesc& desc, GraphicsPipeline* pipeline);
	//void create_compute_pipeline(ComputePipeline* pipeline);



	/*
		A draw is expected to be done between a begin_pass and end_pass!
		Otherwise, no Render Targets are set (D3D11 will complain)
	*/

	void begin_pass(const Framebuffer* framebuffer, DepthStencilClear ds_clear = DepthStencilClear::d1_s0());
	void end_pass();
	
	/*
		in the spirit of using custom types, make custom types for Viewport and Rect!
	*/
	void bind_viewports(const std::vector<D3D11_VIEWPORT>& viewports);
	void bind_scissors(const std::vector<D3D11_RECT>& rects);
	
	// No need to check for type, D3D11 will do it for us
	void bind_vertex_buffer(UINT slot, const GPUBuffer* buffer, UINT stride, UINT offset);
	void bind_index_buffer(const GPUBuffer* buffer, DXGI_FORMAT format, UINT offset);
	void bind_constant_buffer(UINT slot, ShaderStage stage, const GPUBuffer* buffer);

	// Bind shader resource (Read access), no need to known underlying type
	void bind_resource(UINT slot, ShaderStage stage, const GPUResource* resource);

	// Bind UAV (Read-Write access) (only supports CS at the moment, 11.1)
	void bind_resource_rw(UINT slot, ShaderStage stage, const GPUResource* resource);

	// Sampler
	void bind_sampler(UINT slot, ShaderStage stage, const Sampler* sampler);

	// Bind helpers
	void bind_pipeline(const GraphicsPipeline* pipeline, std::array<FLOAT, 4> blend_factor = { 1.f, 1.f, 1.f, 1.f }, UINT stencil_ref = 0);
	void bind_compute_pipeline(const ComputePipeline* pipeline);

	/*
		
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
	static void initialize(unique_ptr<DXDevice> dev);
	static void shutdown();
	static GfxDevice* get();

	GfxDevice() = delete;
	GfxDevice(unique_ptr<class DXDevice> dev);
	~GfxDevice();

	GfxDevice& operator=(const GfxDevice&) = delete;
	GfxDevice(const GfxDevice&) = delete;

private:

	unique_ptr<DXDevice> m_dev;
	GPUTexture m_backbuffer;

};




