#pragma once
#include "Graphics/API/GfxCommon.h"
#include "Graphics/API/GfxDescriptorsPrimitive.h"
#include "Graphics/API/GfxDescriptorsAbstraction.h"
#include "Graphics/API/GfxVertexTypes.h"
#include "Graphics/API/GfxTypes.h"
#include "Graphics/API/GfxHelperTypes.h"
#include "Profiler/GPUProfiler.h"
#include "ResourceHandleStack.h"

#include "Graphics/API/GfxHandles.h"

#include <array>

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


	void create_texture(const TextureDesc& desc, GPUTexture* texture, std::optional<SubresourceData> subres = {});



	BufferHandle create_buffer_2(const BufferDesc& desc, std::optional<SubresourceData> subres = {});
	TextureHandle create_texture_2(const TextureDesc& desc, std::optional<SubresourceData> subres = {});
	PipelineHandle create_pipeline_2(const PipelineDesc& desc);
	RenderPassHandle create_renderpass_2(const FramebufferDesc& desc);

	ShaderHandle compile_and_create_shader(ShaderStage stage, const std::filesystem::path& fname);
	ShaderHandle create_shader_2(ShaderStage stage, const ShaderBytecode& bytecode);
	SamplerHandle create_sampler_2(const SamplerDesc& desc);
	

	void bind_pipeline_2(PipelineHandle pipeline, std::array<FLOAT, 4> blend_factor = { 1.f, 1.f, 1.f, 1.f }, UINT stencil_ref = 0);
	void bind_vertex_buffers_2(UINT start_slot, const std::array<std::tuple<BufferHandle, UINT, UINT>, 12> buffers_strides_offsets);
	void bind_index_buffer_2(BufferHandle buffer, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT, UINT offset = 0);

	void map_copy_2(BufferHandle dst, const SubresourceData& data, D3D11_MAP map_type = D3D11_MAP_WRITE_DISCARD, UINT dst_subres_idx = 0);
	void map_copy_2(TextureHandle dst, const SubresourceData& data, D3D11_MAP map_type = D3D11_MAP_WRITE_DISCARD, UINT dst_subres_idx = 0);

	// copies CPU data to subres at non-mappable memory
	void update_subresource(BufferHandle dst, const SubresourceData& data, const D3D11_BOX& dst_box, UINT dst_subres_idx = 0);
	void update_subresource(TextureHandle dst, const SubresourceData& data, const D3D11_BOX& dst_box, UINT dst_subres_idx = 0);

	// https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0
	// Use the giant constant buffer thingy for per draw data! (not instancing, thats a different buffer which is passed through VB)
	// We have 256 bytes per draw (16 floats!) 
	void bind_constant_buffer_2(UINT slot, ShaderStage stage, BufferHandle buffer, UINT offset_256s = 0, UINT range_256s = 1);



	void bind_resource(UINT slot, ShaderStage stage, BufferHandle resource);
	void bind_resource(UINT slot, ShaderStage stage, TextureHandle resource);
	void bind_resource_rw(UINT slot, ShaderStage stage, BufferHandle resource, UINT initial_count);
	void bind_resource_rw(UINT slot, ShaderStage stage, TextureHandle resource, UINT initial_count);


	void bind_sampler_2(UINT slot, ShaderStage stage, SamplerHandle sampler);

	void begin_pass(RenderPassHandle rp, DepthStencilClear ds_clear = DepthStencilClear::d1_s0());


	//void create_compute_pipeline(ComputePipeline* pipeline);

	/*
	void free_buffer(BufferHandle hdl);
	void free_texture(TextureHandle hdl);
	void free_sampler(SamplerHandle hdl);
	void free_shader(ShaderHandle hdl);
	void free_pipeline(PipelineHandle hdl);
	
	*/


	// No need to change
	void bind_viewports(const std::vector<D3D11_VIEWPORT>& viewports);
	void bind_scissors(const std::vector<D3D11_RECT>& rects);

	//void bind_compute_pipeline(const ComputePipeline* pipeline);
	void dispatch(UINT blocks_x, UINT blocks_y, UINT blocks_z);
	void end_pass();


	void draw(UINT vertex_count, UINT start_loc = 0);
	void draw_indexed(UINT index_count, UINT index_start = 0, UINT vertex_start = 0);
	void present(bool vsync = true);






	// To fix
	void begin_pass(const Framebuffer* framebuffer, DepthStencilClear ds_clear = DepthStencilClear::d1_s0());

	// Removing these needs refactoring on Model
	void bind_resource(UINT slot, ShaderStage stage, const GPUResource* resource);
	void bind_vertex_buffers(UINT start_slot, UINT count, const GPUBuffer* buffers, const UINT* strides, const UINT* offsets = nullptr);
	void bind_index_buffer(const GPUBuffer* buffer, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT, UINT offset = 0);

	// Removing these needs refactoring on Managers
	void create_buffer(const BufferDesc& desc, GPUBuffer* buffer, std::optional<SubresourceData> subres = {});

private:

	void compile_and_create_shader(ShaderStage stage, const std::filesystem::path& fname, Shader* shader, bool recompilation = false);
	void compile_shader(ShaderStage stage, const std::filesystem::path& fname, ShaderBytecode* bytecode, bool recompilation);
	void create_shader(ShaderStage stage, const ShaderBytecode& bytecode, Shader* shader);
	void create_framebuffer(const FramebufferDesc& desc, Framebuffer* framebuffer);
	void bind_resource_rw(UINT slot, ShaderStage stage, const GPUResource* resource, UINT initial_count);

	void create_sampler(const SamplerDesc& desc, Sampler* sampler);
	void bind_sampler(UINT slot, ShaderStage stage, const Sampler* sampler);


	void bind_constant_buffer(UINT slot, ShaderStage stage, const GPUBuffer* buffer, UINT offset_256s = 0, UINT range_256s = 1);
	void update_subresource(const GPUResource* dst, const SubresourceData& data, const D3D11_BOX& dst_box, UINT dst_subres_idx = 0);
	void create_pipeline(const PipelineDesc& desc, GraphicsPipeline* pipeline);
	void bind_pipeline(const GraphicsPipeline* pipeline, std::array<FLOAT, 4> blend_factor = { 1.f, 1.f, 1.f, 1.f }, UINT stencil_ref = 0);
	void map_copy(const GPUResource* dst, const SubresourceData& data, D3D11_MAP map_type = D3D11_MAP_WRITE_DISCARD, UINT dst_subres_idx = 0);



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

	// Should be refactored into a (We should start refatoring loaded pipelines using the new handles! (small surface area))
	std::map<std::string, std::vector<PipelineHandle>> m_loaded_pipelines;
	bool m_reloading_on = true;

	// Resource storage (reasonable max limits hardcoded)
	static constexpr uint64_t MAX_SAMPLER_STORAGE = 32;
	static constexpr uint64_t MAX_FRAMEBUFFER_STORAGE = 256;
	static constexpr uint64_t MAX_PIPELINE_STORAGE = 1024;
	static constexpr uint64_t MAX_SHADER_STORAGE = 256;

	ResourceHandleStack<GPUBuffer> m_buffers;
	ResourceHandleStack<GPUTexture> m_textures;
	ResourceHandleStack<Sampler, MAX_SAMPLER_STORAGE> m_samplers;
	ResourceHandleStack<Framebuffer, MAX_FRAMEBUFFER_STORAGE> m_framebuffers;
	ResourceHandleStack<GraphicsPipeline, MAX_PIPELINE_STORAGE> m_pipelines;
	ResourceHandleStack<Shader, MAX_SHADER_STORAGE> m_shaders;



};






