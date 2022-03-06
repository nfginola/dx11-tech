#pragma once

// Exposing only required dependencies and keeping types as internal implementation
#include "Graphics/API/GfxCommon.h"
#include "Graphics/API/GfxDescriptorsPrimitive.h"
#include "Graphics/API/GfxDescriptorsAbstraction.h"
#include "Graphics/API/GfxHelperTypes.h"
#include "Graphics/API/GfxHandles.h"
#include "Profiler/GPUProfiler.h"
#include "ResourceHandlePool.h"

#include <array>
#include <variant>

struct GPUType;
struct GPUResource;
struct GPUTexture;
struct GPUBuffer;
struct RenderPass;
struct GraphicsPipeline;
struct ComputePipeline;
struct Shader;
struct Sampler;

/*	
	This API is D3D11 ONLY and is meant for:
		- Exploration (exposing the learning "surface area" in a controlled manner)
		- Techniques (using the new learned tools for practical purposes)

	This API leverages the old slot-binding model for simplicity and aims to be as stateless as possible.
	Although, states which make practical sense such as persistent samplers will be utilized.

*/
class GfxDevice
{
	friend class ImGuiDevice;
public:
	// Book-keeping (e.g cleanup)
	void frame_start();	
	void frame_end();

	// Helpers
	TextureHandle get_backbuffer();
	GPUProfiler* get_profiler();
	GPUAnnotator* get_annotator();

	std::pair<UINT, UINT> get_sc_dim();
	void resize_swapchain(UINT width, UINT height);
	void set_name(BufferHandle res, const std::string& name);
	void set_name(TextureHandle res, const std::string& name);
	void set_name(SamplerHandle res, const std::string& name);

	/*
		This should eventually be placed inside but in another class, e.g PipelineManager.
		We can:
			- Store map<shaderName, std::vector<PipelineHandle>>
				- Use a std::multimap instead https://www.fluentcpp.com/2018/04/10/maps-vectors-multimap/
			- Have access to the implementation loaded Pipelines

		For a shader name:
			For a pipeline associated with that shader name:
				Get internal pipeline data (e.g filepaths)
				Recompile Pipeline
	
	*/
	void recompile_pipeline_shaders_by_name(const std::string& name);

	// Resource creation
	BufferHandle create_buffer(const BufferDesc& desc, std::optional<SubresourceData> subres = {});
	TextureHandle create_texture(const TextureDesc& desc, std::optional<SubresourceData> subres = {});
	PipelineHandle create_pipeline(const PipelineDesc& desc);
	ComputePipelineHandle create_compute_pipeline(const ComputePipelineDesc& desc);
	RenderPassHandle create_renderpass(const RenderPassDesc& desc);

	ShaderHandle compile_and_create_shader(ShaderStage stage, const std::filesystem::path& fname);
	ShaderHandle create_shader(ShaderStage stage, const ShaderBytecode& bytecode);
	SamplerHandle create_sampler(const SamplerDesc& desc);

	// Resource destruction
	void free_buffer(BufferHandle hdl);
	void free_texture(TextureHandle hdl);
	void free_sampler(SamplerHandle hdl);
	void free_shader(ShaderHandle hdl);
	void free_pipeline(PipelineHandle hdl);
	void free_renderpass(RenderPassHandle hdl);
	
	// Binds
	void bind_compute_pipeline(ComputePipelineHandle pipeline);
	void bind_pipeline(PipelineHandle pipeline, std::array<FLOAT, 4> blend_factor = { 1.f, 1.f, 1.f, 1.f }, UINT stencil_ref = 0);
	void begin_pass(RenderPassHandle rp, DepthStencilClear ds_clear = DepthStencilClear::d1_s0());
	void end_pass();

	// https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0
	// Use the giant constant buffer thingy for per draw data! (not instancing, thats a different buffer which is passed through VB)
	// We have 256 bytes per draw (16 floats!) 
	void bind_constant_buffer(UINT slot, ShaderStage stage, BufferHandle buffer, UINT offset256s = 0, UINT range256s = 1);
	void bind_vertex_buffers(UINT start_slot, const std::vector<std::tuple<BufferHandle, UINT, UINT>>& buffers_strides_offsets);
	void bind_vertex_buffers(UINT start_slot, void* buffers_strides_offsets, uint8_t count);
	void bind_index_buffer(BufferHandle buffer, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT, UINT offset = 0);

	void bind_resource(UINT slot, ShaderStage stage, BufferHandle resource);
	void bind_resource(UINT slot, ShaderStage stage, TextureHandle resource);
	void bind_sampler(UINT slot, ShaderStage stage, SamplerHandle sampler);
	void bind_resource_rw(UINT slot, ShaderStage stage, BufferHandle resource, UINT initial_count);
	void bind_resource_rw(UINT slot, ShaderStage stage, TextureHandle resource, UINT initial_count);

	void bind_viewports(const std::vector<D3D11_VIEWPORT>& viewports);
	void bind_scissors(const std::vector<D3D11_RECT>& rects);

	// Copies CPU data to mappable memory
	void map_copy(BufferHandle dst, const SubresourceData& data, D3D11_MAP map_type = D3D11_MAP_WRITE_DISCARD, UINT dst_subres_idx = 0);
	void map_copy(TextureHandle dst, const SubresourceData& data, D3D11_MAP map_type = D3D11_MAP_WRITE_DISCARD, UINT dst_subres_idx = 0);

	// Copies CPU data to subres at non-mappable memory
	void update_subresource(BufferHandle dst, const SubresourceData& data, const D3D11_BOX& dst_box, UINT dst_subres_idx = 0);
	void update_subresource(TextureHandle dst, const SubresourceData& data, const D3D11_BOX& dst_box, UINT dst_subres_idx = 0);

	void dispatch(UINT blocks_x, UINT blocks_y, UINT blocks_z);
	void draw(UINT vertex_count, UINT start_loc = 0);
	void draw_indexed(UINT index_count, UINT index_start = 0, UINT vertex_start = 0);
	void present(bool vsync = true);


	/*

	Priority implement:
		DrawIndexedInstanced

	Second prio:
		Begin
		End
		GetData
		SetPredication
		GetPredication
		Dispatch

	Back Burner:
		void clear_readwrite_resource(const GPUResource* resource, ReadWriteClear clear);
	
		
		https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-copyresource
		Docs say that the type of resource of SRC and DST have to be the SAME, meaning it perfectly fits with our 
		Buffer/Texture strongly typed interface.

		void copy_resource_full(BufferHandle dst, BufferHandle src);
		void copy_resource_full(TextureHandle dst, TextureHandle src);

		// args from: https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-copysubresourceregion
		void copy_resource_region(TextureCopyRegionDest dst, TextureCopyRegionSrc src);
		void copy_resource_region(BufferCopyRegionDest dst, BufferCopyRegionSrc src);

		
		
		ID3D11DeviceContext::CopySubresourceRegion

		GetResourceMinLOD??

		DispatchIndirect
		DrawAuto
		DrawIndexed
		DrawIndexedInstancedIndirect
		DrawInstanced
		DrawInstancedIndirect
	*/



	// Helper implementations
private:

	void create_texture(const TextureDesc& desc, GPUTexture* texture, std::optional<SubresourceData> subres = {});
	void create_buffer(const BufferDesc& desc, GPUBuffer* buffer, std::optional<SubresourceData> subres = {});
	void create_shader(ShaderStage stage, const ShaderBytecode& bytecode, Shader* shader);
	void create_sampler(const SamplerDesc& desc, Sampler* sampler);
	void compile_shader(ShaderStage stage, const std::filesystem::path& fname, ShaderBytecode* bytecode, bool recompilation);
	void compile_and_create_shader(ShaderStage stage, const std::filesystem::path& fname, Shader* shader, bool recompilation = false);
	void create_pipeline(const PipelineDesc& desc, GraphicsPipeline* pipeline);
	void create_renderpass(const RenderPassDesc& desc, RenderPass* RenderPass);

	void begin_pass(const RenderPass* RenderPass, DepthStencilClear ds_clear = DepthStencilClear::d1_s0());

	void bind_resource(UINT slot, ShaderStage stage, const GPUResource* resource);
	void bind_resource_rw(UINT slot, ShaderStage stage, const GPUResource* resource, UINT initial_count);
	void bind_sampler(UINT slot, ShaderStage stage, const Sampler* sampler);
	void bind_constant_buffer(UINT slot, ShaderStage stage, const GPUBuffer* buffer, UINT offset256s = 0, UINT range256s = 1);
	void bind_pipeline(const GraphicsPipeline* pipeline, std::array<FLOAT, 4> blend_factor = { 1.f, 1.f, 1.f, 1.f }, UINT stencil_ref = 0);
	
	void update_subresource(const GPUResource* dst, const SubresourceData& data, const D3D11_BOX& dst_box, UINT dst_subres_idx = 0);
	void map_copy(const GPUResource* dst, const SubresourceData& data, D3D11_MAP map_type = D3D11_MAP_WRITE_DISCARD, UINT dst_subres_idx = 0);

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
	TextureHandle m_backbuffer;
	unique_ptr<GPUProfiler> m_profiler;
	unique_ptr<GPUAnnotator> m_annotator;

	// Raster UAVs (bindable to VS, DS, HS, GS, PS)
	std::array<ID3D11UnorderedAccessView*, gfxconstants::MAX_BOUND_UAVS> m_raster_uavs;
	std::array<UINT, gfxconstants::MAX_BOUND_UAVS> m_raster_uav_initial_counts;
	UINT m_raster_rw_range_this_pass = 0;

	// State
	bool m_inside_pass = false;
	const RenderPass* m_active_rp = nullptr;
	const GraphicsPipeline* m_curr_pipeline = nullptr;

	// Redundant state filtering
	/*
		Since we are going as stateless as possible with our workflow.
		Submitted draws will likely have duplicate resources.
		Hence by doing filtering here, higher level code can freely submit
		all required resources for every draw without having to worry about
		rebinding of the same resources 
		(given that draws have been sorted in some order to 'chunk' state variations)
	*/
	// 6 is the shader stage count
	std::array<std::array<SamplerHandle, gfxconstants::MAX_SAMPLERS>, 6> m_bound_samplers;
	std::array<std::array<std::tuple<BufferHandle, UINT, UINT>, gfxconstants::MAX_CB_SLOTS>, 6>  m_bound_cbuffers;
	std::array<std::array<TextureHandle, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS>, 6>  m_bound_read_textures;
	std::array<std::array<BufferHandle, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS>, 6>  m_bound_read_bufs;

	std::array<BufferHandle, gfxconstants::MAX_INPUT_SLOTS> m_bound_vbs;
	BufferHandle m_bound_ib;

	/*
		TO-DO: We should create Common Samplers which we can return. (Check DXTK for common samplers reference)

		using CommonSamplers = std::map<std::string, SamplerHandle>;
		CommonSamplers get_common_samplers();

	*/

	// Resource storage (reasonable max limits hardcoded)
	static constexpr uint64_t MAX_SAMPLER_STORAGE = 64;
	static constexpr uint64_t MAX_RENDERPASS_STORAGE = 256;
	static constexpr uint64_t MAX_PIPELINE_STORAGE = 1024;
	static constexpr uint64_t MAX_COMPUTE_PIPELINES = 64;
	static constexpr uint64_t MAX_SHADER_STORAGE = 256;

	ResourceHandlePool<GPUBuffer> m_buffers;
	ResourceHandlePool<GPUTexture> m_textures;
	ResourceHandlePool<Sampler, MAX_SAMPLER_STORAGE> m_samplers;
	ResourceHandlePool<RenderPass, MAX_RENDERPASS_STORAGE> m_renderpasses;
	ResourceHandlePool<GraphicsPipeline, MAX_PIPELINE_STORAGE> m_pipelines;
	ResourceHandlePool<ComputePipeline, MAX_COMPUTE_PIPELINES> m_compute_pipelines;
	ResourceHandlePool<Shader, MAX_SHADER_STORAGE> m_shaders;

	// Pipeline reloading by shader name (should be refactored into some PipelineManager)
	std::map<std::string, std::vector<PipelineHandle>> m_loaded_pipelines;
	std::map<std::string, std::vector<ComputePipelineHandle>> m_loaded_compute_pipelines;
	bool m_reloading_on = true;
};






