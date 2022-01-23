#pragma once
#include "Graphics/GfxCommon.h"
#include "Graphics/GfxDescriptorsPrimitive.h"
#include "Graphics/GfxDescriptorsAbstraction.h"
#include "Graphics/GfxVertexTypes.h"
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
	/*
		To-do
			- Pipeline Cache
				- When binding Pipeline, check for already bound state.
				- have m_prev_pipeline pointer and compare internals
	*/

	// Book-keeping (e.g cleanup)
	void frame_start();	
	void frame_end();

	// Helpers
	GPUTexture* get_backbuffer();
	void compile_and_create_shader(ShaderStage stage, const std::filesystem::path& fpath, Shader* shader);
	void compile_shader(ShaderStage stage, const std::filesystem::path& fpath, ShaderBytecode* bytecode);

	/*
	Idea:
		To solve dynamically binding a resource with different views (e.g accessing different subresources),
		We can add a create resource function which overrides views:

			create_texture_access(GPUTexture* access_tex, const GPUTexture* src_tex, const ViewDescriptor& desc);
				.. get internal tex from GPUTexture..
				.. branch on bind flags ..
					.. get_srv_format(tex1/2/3d_desc) or get_rtv_format(tex1/2/3d_desc) (extract function from create_texture)
					.. create uav/srv with slicing/indexing Params from ViewDescriptor

			This will return a texture that uses the same underlying texture but is accessed differently.

			Make sure it is clear that this is OPTIONAL!
	*/
	//void create_special_resource_access(const ViewDesc& desc, GPUResource* dst, const GPUResource* src);

	void create_buffer(const BufferDesc& desc, GPUBuffer* buffer, std::optional<SubresourceData> subres = {});
	void create_texture(const TextureDesc& desc, GPUTexture* texture, std::optional<SubresourceData> subres = {});
	void create_sampler(const SamplerDesc& desc, Sampler* sampler);
	void create_shader(ShaderStage stage, const ShaderBytecode& bytecode, Shader* shader);
	void create_framebuffer(const FramebufferDesc& desc, Framebuffer* framebuffer);
	void create_pipeline(const PipelineDesc& desc, GraphicsPipeline* pipeline);
	//void create_compute_pipeline(ComputePipeline* pipeline);
	// How the hell do you know when it is safe to Unbind UAVs from Compute Shader???
	// https://stackoverflow.com/questions/55005420/how-to-do-a-blocking-wait-for-a-compute-shader-with-direct3d11

	//void bind_compute_pipeline(const ComputePipeline* pipeline);
	void bind_pipeline(const GraphicsPipeline* pipeline, std::array<FLOAT, 4> blend_factor = { 1.f, 1.f, 1.f, 1.f }, UINT stencil_ref = 0);
	void bind_vertex_buffers(UINT start_slot, UINT count, const GPUBuffer* buffers, UINT* strides, UINT* offsets = nullptr);
	void bind_index_buffer(const GPUBuffer* buffer, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT, UINT offset = 0);
	void bind_constant_buffer(UINT slot, ShaderStage stage, const GPUBuffer* buffer);
	void bind_resource(UINT slot, ShaderStage stage, const GPUResource* resource);
	void bind_resource_rw(UINT slot, ShaderStage stage, const GPUResource* resource, UINT initial_count);
	void bind_sampler(UINT slot, ShaderStage stage, const Sampler* sampler);
	void bind_viewports(const std::vector<D3D11_VIEWPORT>& viewports);
	void bind_scissors(const std::vector<D3D11_RECT>& rects);



	void begin_pass(const Framebuffer* framebuffer, DepthStencilClear ds_clear = DepthStencilClear::d1_s0());
	void end_pass();

	void draw(UINT vertex_count, UINT start_loc = 0);
	void draw_indexed(UINT index_count, UINT index_start = 0, UINT vertex_start = 0);
	void present(bool vsync = true);

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

	std::array<ID3D11UnorderedAccessView*, gfxconstants::MAX_RASTER_UAVS> m_raster_uavs;
	std::array<UINT, gfxconstants::MAX_RASTER_UAVS> m_raster_uav_initial_counts;
	UINT m_raster_rw_range_this_pass = 0;

	bool m_inside_pass = false;

	const Framebuffer* m_active_framebuffer = nullptr;


};




