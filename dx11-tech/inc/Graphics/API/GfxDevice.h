#pragma once
#include "Graphics/API/GfxCommon.h"
#include "Graphics/API/GfxDescriptorsPrimitive.h"
#include "Graphics/API/GfxDescriptorsAbstraction.h"
#include "Graphics/API/GfxVertexTypes.h"
#include "Graphics/API/GfxTypes.h"
#include "Graphics/API/GfxHelperTypes.h"
#include "Profiler/GPUProfiler.h"
#include "ResourceHandleStack.h"

/*
	Once performance has been measured, only then should we allow binding multiple resources instead of single slot bindings.
	We need to explore GPU Queries before we make any optimizations.
	
	This API is D3D11 ONLY and is meant for:
		- Exploration (exposing the learning "surface area" in a controlled manner)
		- Techniques (using the new learned tools for practical purposes)

*/
class GfxDevice
{
	friend class ImGuiDevice;
public:
	//static GfxDevice* dev;

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
	GPUProfiler* get_profiler();
	GPUAnnotator* get_annotator();

	std::pair<UINT, UINT> get_sc_dim();
	void resize_swapchain(UINT width, UINT height);

	void compile_and_create_shader(ShaderStage stage, const std::filesystem::path& fname, Shader* shader);
	void compile_shader(ShaderStage stage, const std::filesystem::path& fname, ShaderBytecode* bytecode);

	void set_name(const GPUType* device_child, const std::string& name);

	/*
		This function may fail if a pipeline previously created is dropped.
		GfxDevice holds a non-owning pointer to the pipelines created, meaning it 
		is the users responsibility to keep them alive.
	
		At the moment, no pipelines are allowed to die for this helper to work.

		We should create a PipelineManager where we store the map.
		And expose the interfaces:
			- create_pipeline(desc, name)				--> Pipeline stored internally in map		(string, Pipeline)
			- recompile_pipeline_shaders_by_name		--> Map with shader names to pipelines		(string, vector<Pipeline*>)
			- get(name)									--> Get named pipeline

	*/
	void recompile_pipeline_shaders_by_name(const std::string& name);

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

	// copies CPU data to subres at non-mappable memory
	void update_subresource(const GPUResource* dst, const SubresourceData& data, const D3D11_BOX& dst_box, UINT dst_subres_idx = 0);
	void map_copy(const GPUResource* dst, const SubresourceData& data, D3D11_MAP map_type = D3D11_MAP_WRITE_DISCARD, UINT dst_subres_idx = 0);

	//void bind_compute_pipeline(const ComputePipeline* pipeline);
	void bind_pipeline(const GraphicsPipeline* pipeline, std::array<FLOAT, 4> blend_factor = { 1.f, 1.f, 1.f, 1.f }, UINT stencil_ref = 0);
	void bind_vertex_buffers(UINT start_slot, UINT count, const GPUBuffer* buffers, const UINT* strides, const UINT* offsets = nullptr);
	void bind_index_buffer(const GPUBuffer* buffer, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT, UINT offset = 0);

	// https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0
	// Use the giant constant buffer thingy for per draw data! (not instancing, thats a different buffer which is passed through VB)
	// We have 256 bytes per draw (16 floats!) 
	void bind_constant_buffer(UINT slot, ShaderStage stage, const GPUBuffer* buffer, UINT offset_256s = 0, UINT range_256s = 1);
	
	void bind_resource(UINT slot, ShaderStage stage, const GPUResource* resource);
	void bind_resource_rw(UINT slot, ShaderStage stage, const GPUResource* resource, UINT initial_count);
	void bind_sampler(UINT slot, ShaderStage stage, const Sampler* sampler);
	void bind_viewports(const std::vector<D3D11_VIEWPORT>& viewports);
	void bind_scissors(const std::vector<D3D11_RECT>& rects);


	void dispatch(UINT blocks_x, UINT blocks_y, UINT blocks_z);
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
	static void initialize(unique_ptr<DXDevice> dx_device);
	static void shutdown();

	GfxDevice() = delete;
	GfxDevice(unique_ptr<class DXDevice> dev);
	~GfxDevice();

	GfxDevice& operator=(const GfxDevice&) = delete;
	GfxDevice(const GfxDevice&) = delete;

private:
	// Backend device
	unique_ptr<DXDevice> m_dev;

	// Miscellaneous
	GPUTexture m_backbuffer;
	unique_ptr<GPUProfiler> m_profiler;
	unique_ptr<GPUAnnotator> m_annotator;

	// Raster UAVs (bindable to VS, DS, HS, GS, PS)
	std::array<ID3D11UnorderedAccessView*, gfxconstants::MAX_RASTER_UAVS> m_raster_uavs;
	std::array<UINT, gfxconstants::MAX_RASTER_UAVS> m_raster_uav_initial_counts;
	UINT m_raster_rw_range_this_pass = 0;

	// State
	bool m_inside_pass = false;
	const Framebuffer* m_active_framebuffer = nullptr;
	const GraphicsPipeline* m_curr_pipeline = nullptr;

	// Should be refactored into a 
	std::map<std::string, std::vector<GraphicsPipeline*>> m_loaded_pipelines;
	bool m_reloading_on = true;

	// Resource storage (reasonable max limits hardcoded)
	static constexpr uint64_t MAX_SAMPLER_STORAGE = 32;
	static constexpr  uint64_t MAX_FRAMEBUFFER_STORAGE = 256;
	static constexpr  uint64_t MAX_PIPELINE_STORAGE = 1024;

	std::unique_ptr<ResourceHandleStack<GPUBuffer>> m_buffers;
	std::unique_ptr<ResourceHandleStack<GPUTexture>> m_textures;
	std::unique_ptr<ResourceHandleStack<Sampler, MAX_SAMPLER_STORAGE>> m_samplers;
	std::unique_ptr<ResourceHandleStack<Framebuffer, MAX_FRAMEBUFFER_STORAGE>> m_framebuffers;
	std::unique_ptr<ResourceHandleStack<GraphicsPipeline, MAX_PIPELINE_STORAGE>> m_pipelines;



};






