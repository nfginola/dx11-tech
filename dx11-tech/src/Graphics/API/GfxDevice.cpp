#include "pch.h"
#include "Graphics/API/GfxDevice.h"
#include "Graphics/API/GfxTypes.h"
#include "Profiler/FrameProfiler.h"

// Globals
namespace gfx
{ 
	GfxDevice* dev = nullptr;  
	GPUAnnotator* annotator = nullptr;
}

//namespace perf
//{
//	extern FrameProfiler* profiler;
//}

namespace gfxconstants
{
	// For unbinding state
	const void* const NULL_RESOURCE[gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS] = {};
}


void GfxDevice::initialize(unique_ptr<DXDevice> dx_device)
{
	//if (!s_gfx_device)
	//	s_gfx_device = new GfxDevice(std::move(dev));
	//else
	//	assert(false);	// dont try initializing multiple times..

	if (!gfx::dev)
		gfx::dev = new GfxDevice(std::move(dx_device));
	else
		assert(false);	// dont try initializing multiple times..

	gfx::annotator = gfx::dev->get_annotator();
}

void GfxDevice::shutdown()
{
	//if (s_gfx_device)
	//	delete s_gfx_device;

	if (gfx::dev)
	{
		delete gfx::dev;
		gfx::dev = nullptr;
	}
	gfx::annotator = nullptr;
}



GfxDevice::GfxDevice(std::unique_ptr<DXDevice> dev) :
	m_dev(std::move(dev))
{
	// Initialize backbuffer texture primitive
	auto [bb_hdl, bb_res] = m_textures.get_next_free_handle();
	bb_res->handle = bb_hdl;
	bb_res->m_internal_resource = m_dev->get_bb_texture();
	bb_res->m_rtv = m_dev->get_bb_target();
	bb_res->m_type = TextureType::e2D;
	m_backbuffer = TextureHandle{ bb_hdl };

	// Initialize annotator
	m_annotator = make_unique<GPUAnnotator>(m_dev->get_annotation());

	// Initialize profiler
	m_profiler = make_unique<GPUProfiler>(m_dev.get());

	uint64_t storage_mem_footprint = 0;
	storage_mem_footprint += m_buffers.get_memory_footprint();
	storage_mem_footprint += m_textures.get_memory_footprint();
	storage_mem_footprint += m_samplers.get_memory_footprint();
	storage_mem_footprint += m_renderpasses.get_memory_footprint();
	storage_mem_footprint += m_pipelines.get_memory_footprint();
	storage_mem_footprint += m_shaders.get_memory_footprint();
	fmt::print("GfxDevice storage memory footprint: {} bytes (~{:.3f} MB)\n", storage_mem_footprint, storage_mem_footprint / (float)10e5);
}

GfxDevice::~GfxDevice()
{

}

void GfxDevice::frame_start()
{
	if (m_profiler)
		m_profiler->frame_start();

	auto& ctx = m_dev->get_context();
	ctx->RSSetScissorRects(gfxconstants::MAX_SCISSORS, (const D3D11_RECT*)gfxconstants::NULL_RESOURCE);

	// nuke all SRVs
	// https://stackoverflow.com/questions/20300778/are-there-directx-guidelines-for-binding-and-unbinding-resources-between-draw-ca
	ctx->VSSetShaderResources(0, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS - 64, (ID3D11ShaderResourceView* const*)gfxconstants::NULL_RESOURCE);
	ctx->HSSetShaderResources(0, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS - 64, (ID3D11ShaderResourceView* const*)gfxconstants::NULL_RESOURCE);
	ctx->DSSetShaderResources(0, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS - 64, (ID3D11ShaderResourceView* const*)gfxconstants::NULL_RESOURCE);
	ctx->GSSetShaderResources(0, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS - 64, (ID3D11ShaderResourceView* const*)gfxconstants::NULL_RESOURCE);
	ctx->PSSetShaderResources(0, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS - 64, (ID3D11ShaderResourceView* const*)gfxconstants::NULL_RESOURCE);
	ctx->CSSetShaderResources(0, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS - 64, (ID3D11ShaderResourceView* const*)gfxconstants::NULL_RESOURCE);

	//ctx->VSSetConstantBuffers(0, gfxconstants::MAX_CB_SLOTS, (ID3D11Buffer* const*)gfxconstants::NULL_RESOURCE);
	//ctx->HSSetConstantBuffers(0, gfxconstants::MAX_CB_SLOTS, (ID3D11Buffer* const*)gfxconstants::NULL_RESOURCE);
	//ctx->DSSetConstantBuffers(0, gfxconstants::MAX_CB_SLOTS, (ID3D11Buffer* const*)gfxconstants::NULL_RESOURCE);
	//ctx->GSSetConstantBuffers(0, gfxconstants::MAX_CB_SLOTS, (ID3D11Buffer* const*)gfxconstants::NULL_RESOURCE);
	//ctx->PSSetConstantBuffers(0, gfxconstants::MAX_CB_SLOTS, (ID3D11Buffer* const*)gfxconstants::NULL_RESOURCE);
	//ctx->CSSetConstantBuffers(0, gfxconstants::MAX_CB_SLOTS, (ID3D11Buffer* const*)gfxconstants::NULL_RESOURCE);
}

void GfxDevice::frame_end()
{	
	{
		//std::memset(m_bound_samplers.data(), 0, sizeof(SamplerHandle) * gfxconstants::MAX_SAMPLERS * 6);
		//std::memset(m_bound_cbuffers.data(), 0, sizeof(std::tuple<BufferHandle, UINT, UINT>) * gfxconstants::MAX_CB_SLOTS * 6);
		std::memset(m_bound_read_textures.data(), 0, sizeof(TextureHandle) * gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS * 6);
		std::memset(m_bound_read_bufs.data(), 0, sizeof(BufferHandle) * gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS * 6);
	}

	// End GPU profiler
	if (m_profiler)
		m_profiler->frame_end();
}



void GfxDevice::compile_and_create_shader(ShaderStage stage, const std::filesystem::path& fname, Shader* shader, bool recompilation)
{
	ShaderBytecode bc;
	compile_shader(stage, fname, &bc, recompilation);
	create_shader(stage, bc, shader);
}

void GfxDevice::compile_shader(ShaderStage stage, const std::filesystem::path& fname, ShaderBytecode* bytecode, bool recompilation)
{
	std::string target;
	switch (stage)
	{
	case ShaderStage::eVertex:
		target = "vs_5_0";
		break;
	case ShaderStage::ePixel:
		target = "ps_5_0";
		break;
	case ShaderStage::eHull:
		target = "hs_5_0";
		break;
	case ShaderStage::eDomain:
		target = "ds_5_0";
		break;
	case ShaderStage::eGeometry:
		target = "gs_5_0";
		break;
	case ShaderStage::eCompute:
		target = "cs_5_0";
		break;
	}

	// Column major packing generally more efficient
	// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/d3dcompile-constants
	UINT flags = D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	auto new_target = std::string(gfxconstants::SHADER_DIRECTORY) + fname.string();
	auto new_fpath = std::filesystem::path(new_target);

	BlobPtr shader_blob;
	BlobPtr error_blob;
	auto HR = D3DCompileFromFile(
		new_fpath.wstring().c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // This default include handler includes files that are relative to the current directory
		"main",
		target.c_str(),
		flags, 0,
		shader_blob.ReleaseAndGetAddressOf(), error_blob.ReleaseAndGetAddressOf()
	);
	if (FAILED(HR))
	{
		if (error_blob)
		{
			OutputDebugStringA((char*)error_blob->GetBufferPointer());
			std::cout << (char*)error_blob->GetBufferPointer() << "\n";
			if (!recompilation)	
				assert(false);
			return;
		}
	}
	
	bytecode->code = std::make_shared<std::vector<uint8_t>>();
	bytecode->code->resize(shader_blob->GetBufferSize());
	std::memcpy(bytecode->code->data(), shader_blob->GetBufferPointer(), shader_blob->GetBufferSize());
	bytecode->fname = fname.string();
}

void GfxDevice::recompile_pipeline_shaders_by_name(const std::string& name)
{
	if (!m_reloading_on)
		return;

	auto fname = name + std::string(".hlsl");		// append extension

	bool graphics_pipe = false;
	bool compute_pipe = false;
	auto graphics_it = m_loaded_pipelines.find(fname);
	auto compute_it = m_loaded_compute_pipelines.find(fname);

	// find pipelines associated with the shader name
	if (graphics_it != m_loaded_pipelines.end())
	{
		graphics_pipe = true;
	}
	// try looking for compute if it didnt find in normal render pipelines list
	else if (compute_it != m_loaded_compute_pipelines.end())
	{
		compute_pipe = true;
	}
	
	assert(graphics_pipe || compute_pipe);	// if/else with this assertion serves as an XOR: we must find either exactly a gph pipe or compute pipe

	/*
		"Recompile every pipeline".

		It is currently redundantly recompiling shaders. It does it more than once (since its recompiling for every pipeline).
		As recompilation is a cold-feature (for rare debug or rapid experimentation), I'll just ignore this for now.
	*/
	if (graphics_pipe)
	{
		auto& pipelines = graphics_it->second;
		/*
			we recompile everything for every pipeline that is related to the reloaded shader.
			if we e.g change output of VS, changes in HS/DS/GS/PS (input) is likely to be changed as well.
			So lets just recompile em all, even if it is slightly redundant
		*/
		for (auto& p : pipelines)
		{
			auto existing_pipeline = m_pipelines.look_up(p.hdl);

			// get filenames to all shaders in the pipelines
			auto existing_vs = m_shaders.look_up(existing_pipeline->m_vs.hdl);
			auto existing_ps = m_shaders.look_up(existing_pipeline->m_ps.hdl);
			const auto& vs_name = existing_vs->m_blob.fname;
			const auto& ps_name = existing_ps->m_blob.fname;

			// compile shaders
			compile_and_create_shader(ShaderStage::eVertex, vs_name, existing_vs, true);
			compile_and_create_shader(ShaderStage::ePixel, ps_name, existing_ps, true);

			if (existing_pipeline->m_gs.hdl != 0)
			{
				auto existing_gs = m_shaders.look_up(existing_pipeline->m_gs.hdl);
				if (const auto& gs_name = existing_gs->m_blob.fname; !gs_name.empty())
					compile_and_create_shader(ShaderStage::eGeometry, gs_name, existing_gs, true);
			}

			if (existing_pipeline->m_hs.hdl != 0)
			{
				auto existing_hs = m_shaders.look_up(existing_pipeline->m_hs.hdl);
				if (const auto& hs_name = existing_hs->m_blob.fname; !hs_name.empty())
					compile_and_create_shader(ShaderStage::eHull, hs_name, existing_hs, true);
			}

			if (existing_pipeline->m_ds.hdl != 0)
			{
				auto existing_ds = m_shaders.look_up(existing_pipeline->m_ds.hdl);
				if (const auto& ds_name = existing_ds->m_blob.fname; !ds_name.empty())
					compile_and_create_shader(ShaderStage::eDomain, ds_name, existing_ds, true);
			}
		}
	}
	else // compute
	{
		ShaderBytecode bc;
		compile_shader(ShaderStage::eCompute, fname, &bc, true);

		auto& pipelines = compute_it->second;
		for (auto& p : pipelines)
		{
			auto existing_pipeline = m_compute_pipelines.look_up(p.hdl);
			auto existing_cs = m_shaders.look_up(existing_pipeline->cs.hdl);
			
			// simply recreate internal resource without modifying handles
			create_shader(ShaderStage::eCompute, bc, existing_cs);
		}
	}

}



TextureHandle GfxDevice::get_backbuffer()
{
	return m_backbuffer;
}

GPUProfiler* GfxDevice::get_profiler()
{
	return m_profiler.get();
}

GPUAnnotator* GfxDevice::get_annotator()
{
	return m_annotator.get();
}

std::pair<UINT, UINT> GfxDevice::get_sc_dim()
{
	auto sc_desc = m_dev->get_sc_desc();
	return { sc_desc.BufferDesc.Width, sc_desc.BufferDesc.Height };
}

void GfxDevice::resize_swapchain(UINT width, UINT height)
{
	// Release the primitive texture
	auto bb = m_textures.look_up(m_backbuffer.hdl);
	bb->m_internal_resource.ReleaseAndGetAddressOf();
	bb->m_rtv.ReleaseAndGetAddressOf();

	// Recreate
	m_dev->resize_swapchain(width, height);

	// Retrieve new swapchain resources
	bb->m_internal_resource = m_dev->get_bb_texture();
	bb->m_rtv = m_dev->get_bb_target();
	bb->m_type = TextureType::e2D;
}

void set_name_internal(GPUType* device_child, const std::string& name)
{
	HRCHECK(device_child->m_internal_resource->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.data()));
}

void GfxDevice::set_name(BufferHandle res, const std::string& name)
{
	set_name_internal(m_buffers.look_up(res.hdl), name);
}

void GfxDevice::set_name(TextureHandle res, const std::string& name)
{
	set_name_internal(m_textures.look_up(res.hdl), name);
}

void GfxDevice::set_name(SamplerHandle res, const std::string& name)
{
	set_name_internal(m_samplers.look_up(res.hdl), name);
}






ShaderHandle GfxDevice::compile_and_create_shader(ShaderStage stage, const std::filesystem::path& fname)
{
	auto [hdl, shader] = m_shaders.get_next_free_handle();
	shader->handle = hdl;
	compile_and_create_shader(stage, fname, shader);
	return ShaderHandle{ hdl };
}

ShaderHandle GfxDevice::create_shader(ShaderStage stage, const ShaderBytecode& bytecode)
{
	auto [hdl, shader] = m_shaders.get_next_free_handle();
	shader->handle = hdl;
	create_shader(stage, bytecode, shader);
	return ShaderHandle{ hdl };
}

SamplerHandle GfxDevice::create_sampler(const SamplerDesc& desc)
{
	auto [hdl, sampler] = m_samplers.get_next_free_handle();
	sampler->handle = hdl;
	create_sampler(desc, sampler);
	return SamplerHandle{ hdl };
}

TextureHandle GfxDevice::create_texture(const TextureDesc& desc, std::optional<SubresourceData> subres)
{
	auto [hdl, texture] = m_textures.get_next_free_handle();
	texture->handle = hdl;
	create_texture(desc, texture, subres);
	return TextureHandle{ hdl };
}

PipelineHandle GfxDevice::create_pipeline(const PipelineDesc& desc)
{
	auto [hdl, pipeline] = m_pipelines.get_next_free_handle();
	pipeline->handle = hdl;
	create_pipeline(desc, pipeline);

	auto ret_hdl = PipelineHandle{ hdl };

	// Add to lookup for reloading
	if (m_reloading_on)
	{
		// add pipeline to lookup table
		auto load_to_cache = [&](const std::string& fname)
		{
			auto it = m_loaded_pipelines.find(fname);
			if (it != m_loaded_pipelines.end())
				it->second.push_back(ret_hdl);
			else
				m_loaded_pipelines.insert({ fname, { ret_hdl } });
		};

		load_to_cache(m_shaders.look_up(pipeline->m_vs.hdl)->m_blob.fname);
		load_to_cache(m_shaders.look_up(pipeline->m_ps.hdl)->m_blob.fname);

		if (desc.m_gs.has_value())
		{
			pipeline->m_gs = *desc.m_gs;
			if (m_reloading_on)
				load_to_cache(m_shaders.look_up(desc.m_gs->hdl)->m_blob.fname);

		}
		if (desc.m_hs.has_value())
		{
			pipeline->m_hs = *desc.m_hs;
			if (m_reloading_on)
				load_to_cache(m_shaders.look_up(desc.m_hs->hdl)->m_blob.fname);

		}
		if (desc.m_ds.has_value())
		{
			pipeline->m_ds = *desc.m_ds;
			if (m_reloading_on)
				load_to_cache(m_shaders.look_up(desc.m_ds->hdl)->m_blob.fname);
		}
	}

	return ret_hdl;
}

ComputePipelineHandle GfxDevice::create_compute_pipeline(const ComputePipelineDesc& desc)
{
	assert(desc.m_cs.hdl != 0);

	auto [hdl, pipeline] = m_compute_pipelines.get_next_free_handle();
	pipeline->handle = hdl;
	
	pipeline->cs = desc.m_cs;
	pipeline->is_registered = true;

	auto ret_hdl = ComputePipelineHandle{ hdl };

	// Add to lookup for reloading
	if (m_reloading_on)
	{
		// add pipeline to lookup table
		auto load_to_cache = [&](const std::string& fname)
		{
			auto it = m_loaded_compute_pipelines.find(fname);
			if (it != m_loaded_compute_pipelines.end())
				it->second.push_back(ret_hdl);
			else
				m_loaded_compute_pipelines.insert({ fname, { ret_hdl } });
		};

		load_to_cache(m_shaders.look_up(pipeline->cs.hdl)->m_blob.fname);
	}

	return ret_hdl;
}

RenderPassHandle GfxDevice::create_renderpass(const RenderPassDesc& desc)
{
	auto [hdl, rp] = m_renderpasses.get_next_free_handle();
	rp->handle = hdl;
	create_renderpass(desc, rp);
	return RenderPassHandle{ hdl };
}

BufferHandle GfxDevice::create_buffer(const BufferDesc& desc, std::optional<SubresourceData> subres)
{
	auto [hdl, buffer] = m_buffers.get_next_free_handle();
	buffer->handle = hdl;
	create_buffer(desc, buffer, subres);
	return BufferHandle{ hdl };
}




void GfxDevice::free_buffer(BufferHandle hdl)
{
	m_buffers.free_handle(hdl.hdl);
}

void GfxDevice::free_texture(TextureHandle hdl)
{
	m_textures.free_handle(hdl.hdl);
}

void GfxDevice::free_sampler(SamplerHandle hdl)
{
	m_samplers.free_handle(hdl.hdl);
}

void GfxDevice::free_shader(ShaderHandle hdl)
{
	m_shaders.free_handle(hdl.hdl);
}

void GfxDevice::free_pipeline(PipelineHandle hdl)
{
	m_pipelines.free_handle(hdl.hdl);
}

void GfxDevice::free_renderpass(RenderPassHandle hdl)
{
	m_renderpasses.free_handle(hdl.hdl);
}



void GfxDevice::begin_pass(RenderPassHandle rp, DepthStencilClear ds_clear)
{
	begin_pass(m_renderpasses.look_up(rp.hdl), ds_clear);
}

void GfxDevice::end_pass()
{
	assert(m_inside_pass == true);
	m_inside_pass = false;
	auto& ctx = m_dev->get_context();

	// set render targets / raster uavs 
	if (m_raster_rw_range_this_pass > 0)
	{
		ctx->OMSetRenderTargetsAndUnorderedAccessViews(
			gfxconstants::MAX_RENDER_TARGETS, (ID3D11RenderTargetView* const*)gfxconstants::NULL_RESOURCE, nullptr,
			0, gfxconstants::MAX_BOUND_UAVS, (ID3D11UnorderedAccessView* const*)gfxconstants::NULL_RESOURCE, nullptr);
		m_raster_rw_range_this_pass = 0;
	}
	else
	{
		ctx->OMSetRenderTargets(gfxconstants::MAX_RENDER_TARGETS, (ID3D11RenderTargetView* const*)gfxconstants::NULL_RESOURCE, nullptr);
	}


	// resolve any ms targets if any
	if (!m_active_rp->m_resolve_targets.empty())
	{
		for (int i = 0; i < m_active_rp->m_targets.size(); ++i)
		{
			auto src = (ID3D11Texture2D*)m_textures.look_up(std::get<TextureHandle>(m_active_rp->m_targets[i]).hdl)->m_internal_resource.Get();
			auto dst = (ID3D11Texture2D*)m_textures.look_up(m_active_rp->m_resolve_targets[i].hdl)->m_internal_resource.Get();
			auto format = std::get<DXGI_FORMAT>(m_active_rp->m_targets[i]);
			ctx->ResolveSubresource(dst, 0, src, 0, format);
		}
	}

	// TO-DO: resolve depth target using compute shader
	// https://wickedengine.net/2016/11/13/how-to-resolve-an-msaa-depthbuffer/#comments

	ctx->VSSetShaderResources(0, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS, (ID3D11ShaderResourceView* const*)gfxconstants::NULL_RESOURCE);
	ctx->HSSetShaderResources(0, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS, (ID3D11ShaderResourceView* const*)gfxconstants::NULL_RESOURCE);
	ctx->DSSetShaderResources(0, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS, (ID3D11ShaderResourceView* const*)gfxconstants::NULL_RESOURCE);
	ctx->GSSetShaderResources(0, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS, (ID3D11ShaderResourceView* const*)gfxconstants::NULL_RESOURCE);
	ctx->PSSetShaderResources(0, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS, (ID3D11ShaderResourceView* const*)gfxconstants::NULL_RESOURCE);


	m_active_rp = nullptr;
}



void GfxDevice::bind_compute_pipeline(ComputePipelineHandle pipeline)
{
	const auto p = m_compute_pipelines.look_up(pipeline.hdl);
	const auto cs = m_shaders.look_up(p->cs.hdl);
	m_dev->get_context()->CSSetShader((ID3D11ComputeShader*)cs->m_internal_resource.Get(), nullptr, 0);
}

void GfxDevice::bind_pipeline(PipelineHandle pipeline, std::array<FLOAT, 4> blend_factor, UINT stencil_ref)
{
	bind_pipeline(m_pipelines.look_up(pipeline.hdl), blend_factor, stencil_ref);
}

void GfxDevice::bind_vertex_buffers(UINT start_slot, const std::vector<std::tuple<BufferHandle, UINT, UINT>>& buffers_strides_offsets)
{
	if (buffers_strides_offsets.size() == 0)
		return;

	UINT identical = 0;
	for (int i = 0; i < buffers_strides_offsets.size(); ++i)
	{
		const auto& buffer_handle = std::get<BufferHandle>(buffers_strides_offsets[i]);
		if (buffer_handle.hdl == m_bound_vbs[start_slot + i].hdl)
			++identical;
	}
	if (identical == buffers_strides_offsets.size())
		return;


	// Refactor this later. We want to remove the redundant Handle -> GPUBuffer -> D3D11Resource
	assert(buffers_strides_offsets.size() <= 12);
	ID3D11Buffer* vbs[gfxconstants::MAX_INPUT_SLOTS] = {};
	UINT strides[gfxconstants::MAX_INPUT_SLOTS] = {};
	UINT offsets[gfxconstants::MAX_INPUT_SLOTS] = {};
	for (int i = 0; i < buffers_strides_offsets.size(); ++i)
	{
		const auto& buffer_handle = std::get<BufferHandle>(buffers_strides_offsets[i]);
		vbs[i] = (ID3D11Buffer*)(m_buffers.look_up(buffer_handle.hdl)->m_internal_resource.Get());
		strides[i] = std::get<1>(buffers_strides_offsets[i]);
		offsets[i] = std::get<2>(buffers_strides_offsets[i]);

		m_bound_vbs[start_slot + i] = buffer_handle;
	}

	m_dev->get_context()->IASetVertexBuffers(
		start_slot, (UINT)buffers_strides_offsets.size(), 
		vbs,
		strides ? strides : (UINT*)gfxconstants::NULL_RESOURCE,
		offsets ? offsets : (UINT*)gfxconstants::NULL_RESOURCE);

	

}

void GfxDevice::bind_vertex_buffers(UINT start_slot, void* buffers_strides_offsets, uint8_t count)
{
	if (count == 0)
		return;

	struct BSO
	{
		BufferHandle hdl;
		UINT stride;
		UINT offset;
	};

	BSO* bso = (BSO*)buffers_strides_offsets;

	UINT identical = 0;
	for (int i = 0; i < count; ++i)
	{
		const auto& buffer_handle = bso[i].hdl;
		if (buffer_handle.hdl == m_bound_vbs[start_slot + i].hdl)
			++identical;
	}
	if (identical == count)
		return;


	// Refactor this later. We want to remove the redundant Handle -> GPUBuffer -> D3D11Resource
	assert(count <= 12);
	ID3D11Buffer* vbs[gfxconstants::MAX_INPUT_SLOTS] = {};
	UINT strides[gfxconstants::MAX_INPUT_SLOTS] = {};
	UINT offsets[gfxconstants::MAX_INPUT_SLOTS] = {};
	for (int i = 0; i < count; ++i)
	{
		const auto& buffer_handle = bso[i].hdl;
		vbs[i] = (ID3D11Buffer*)(m_buffers.look_up(buffer_handle.hdl)->m_internal_resource.Get());
		strides[i] = bso[i].stride;
		offsets[i] = bso[i].offset;

		m_bound_vbs[start_slot + i] = buffer_handle;
	}

	m_dev->get_context()->IASetVertexBuffers(
		start_slot, (UINT)count,
		vbs,
		strides ? strides : (UINT*)gfxconstants::NULL_RESOURCE,
		offsets ? offsets : (UINT*)gfxconstants::NULL_RESOURCE);
}



void GfxDevice::bind_index_buffer(BufferHandle buffer, DXGI_FORMAT format, UINT offset)
{
	if (buffer.hdl == m_bound_ib.hdl)
		return;
	m_dev->get_context()->IASetIndexBuffer((ID3D11Buffer*)m_buffers.look_up(buffer.hdl)->m_internal_resource.Get(), format, offset);

	m_bound_ib = buffer;
}

void GfxDevice::map_copy(BufferHandle dst, const SubresourceData& data, D3D11_MAP map_type, UINT dst_subres_idx)
{
	map_copy(m_buffers.look_up(dst.hdl), data, map_type, dst_subres_idx);
}

void GfxDevice::map_copy(TextureHandle dst, const SubresourceData& data, D3D11_MAP map_type, UINT dst_subres_idx)
{
	map_copy(m_textures.look_up(dst.hdl), data, map_type, dst_subres_idx);
}

void GfxDevice::update_subresource(BufferHandle dst, const SubresourceData& data, const D3D11_BOX& dst_box, UINT dst_subres_idx)
{
	update_subresource(m_buffers.look_up(dst.hdl), data, dst_box, dst_subres_idx);
}

void GfxDevice::update_subresource(TextureHandle dst, const SubresourceData& data, const D3D11_BOX& dst_box, UINT dst_subres_idx)
{
	update_subresource(m_textures.look_up(dst.hdl), data, dst_box, dst_subres_idx);
}

void GfxDevice::bind_constant_buffer(UINT slot, ShaderStage stage, BufferHandle buffer, UINT offset256s, UINT range256s)
{
	auto& curr_bound = m_bound_cbuffers[(UINT)stage - 1][slot];

	if (std::get<BufferHandle>(curr_bound).hdl == buffer.hdl &&
		std::get<1>(curr_bound) == offset256s &&
		std::get<2>(curr_bound) == range256s)
		return;

	bind_constant_buffer(slot, stage, m_buffers.look_up(buffer.hdl), offset256s, range256s);
	std::get<BufferHandle>(curr_bound) = buffer;
	std::get<1>(curr_bound) = offset256s;
	std::get<2>(curr_bound) = range256s;
}

void GfxDevice::bind_resource(UINT slot, ShaderStage stage, BufferHandle resource)
{
	if (m_bound_read_bufs[(UINT)stage - 1][slot].hdl == resource.hdl)
		return;
	bind_resource(slot, stage, m_buffers.look_up(resource.hdl));
	m_bound_read_bufs[(UINT)stage - 1][slot] = resource;
}

void GfxDevice::bind_resource(UINT slot, ShaderStage stage, TextureHandle resource)
{
	if (m_bound_read_textures[(UINT)stage - 1][slot].hdl == resource.hdl)
		return;
	bind_resource(slot, stage, m_textures.look_up(resource.hdl));
	m_bound_read_textures[(UINT)stage - 1][slot] = resource;
}

void GfxDevice::bind_resource_rw(UINT slot, ShaderStage stage, BufferHandle resource, UINT initial_count)
{
	bind_resource_rw(slot, stage, m_buffers.look_up(resource.hdl), initial_count);
}

void GfxDevice::bind_resource_rw(UINT slot, ShaderStage stage, TextureHandle resource, UINT initial_count)
{
	bind_resource_rw(slot, stage, m_textures.look_up(resource.hdl), initial_count);
}

void GfxDevice::bind_sampler(UINT slot, ShaderStage stage, SamplerHandle sampler)
{
	if (m_bound_samplers[(UINT)stage - 1][slot].hdl == sampler.hdl)
		return;
	bind_sampler(slot, stage, m_samplers.look_up(sampler.hdl));
	m_bound_samplers[(UINT)stage - 1][slot] = sampler;
}

void GfxDevice::bind_viewports(const std::vector<D3D11_VIEWPORT>& viewports)
{
	auto& ctx = m_dev->get_context();
	ctx->RSSetViewports((UINT)viewports.size(), viewports.data());
}

void GfxDevice::bind_scissors(const std::vector<D3D11_RECT>& rects)
{
	auto& ctx = m_dev->get_context();
	ctx->RSSetScissorRects((UINT)rects.size(), rects.data());
}





void GfxDevice::dispatch(UINT blocks_x, UINT blocks_y, UINT blocks_z)
{
	auto& ctx = m_dev->get_context();

	ctx->Dispatch(blocks_x, blocks_y, blocks_z);

	// https://on-demand.gputechconf.com/gtc/2010/presentations/S12312-DirectCompute-Pre-Conference-Tutorial.pdf
	// How do you know when it is safe to Unbind UAVs from Compute Shader???
	// Ans: Check GP Discord, DirectX section, answer from jwki
	// No need to sync with Fence
	// https://stackoverflow.com/questions/55005420/how-to-do-a-blocking-wait-for-a-compute-shader-with-direct3d11
	ctx->CSSetUnorderedAccessViews(0, gfxconstants::MAX_BOUND_UAVS, (ID3D11UnorderedAccessView* const*)gfxconstants::NULL_RESOURCE, (const UINT*)gfxconstants::NULL_RESOURCE);
	ctx->CSSetShaderResources(0, gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS, (ID3D11ShaderResourceView* const*)gfxconstants::NULL_RESOURCE);
}

void GfxDevice::draw(UINT vertex_count, UINT start_loc)
{
	assert(m_inside_pass == true && "Draw call must be inside a Pass scope!");
	m_dev->get_context()->Draw(vertex_count, start_loc);
}

void GfxDevice::draw_indexed(UINT index_count, UINT index_start, UINT vertex_start)
{
	m_dev->get_context()->DrawIndexed(index_count, index_start, vertex_start);
}

void GfxDevice::present(bool vsync)
{
	//m_profiler->begin("Presentation", false, false);
	m_dev->get_sc()->Present(vsync ? 1 : 0, 0);
	//m_profiler->end("Presentation");
}

void GfxDevice::copy_resource_region(BufferHandle dst, const CopyRegionDst& dst_desc, BufferHandle src, const CopyRegionSrc& src_desc)
{
	auto dst_b = (ID3D11Resource*)m_buffers.look_up(dst.hdl)->m_internal_resource.Get();
	auto src_b = (ID3D11Resource*)m_buffers.look_up(src.hdl)->m_internal_resource.Get();

	m_dev->get_context()->CopySubresourceRegion1(
		dst_b, dst_desc.m_subres, dst_desc.m_x, dst_desc.m_y, dst_desc.m_z,
		src_b, src_desc.m_subres, &src_desc.m_box, src_desc.m_copy_flags);
}

std::pair<float, float> GfxDevice::map_read_temp(BufferHandle buf)
{
	auto res = (ID3D11Resource*)m_buffers.look_up(buf.hdl)->m_internal_resource.Get();
	D3D11_MAPPED_SUBRESOURCE subr;
	m_dev->get_context()->Map(res, 0, D3D11_MAP_READ, 0, &subr);

	float minmax[2] = { 0, 0 };
	for (int i = 0; i < 2; ++i)
	{
		minmax[i] = ((float*)subr.pData)[i];
	}

	fmt::print("Min: {:.8f}, Max: {:.8f}\n", minmax[1], minmax[0]);


	m_dev->get_context()->Unmap(res, 0);

	return { minmax[0], minmax[1] };
}










/*
	=========================================	HELPER INTERNAL IMPLEMENTATIONS BELOW    =================================================

*/


void GfxDevice::create_buffer(const BufferDesc& desc, GPUBuffer* buffer, std::optional<SubresourceData> subres)
{
	const auto& d3d_desc = desc.m_desc;

	HRCHECK(m_dev->get_device()->CreateBuffer(
		&d3d_desc,
		subres ? &subres->m_subres : nullptr,
		(ID3D11Buffer**)buffer->m_internal_resource.ReleaseAndGetAddressOf()));

	// constant buffers dont need views
	if (desc.m_type == BufferType::eConstant)
		return;

	// Create views
	if (d3d_desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SRV_DIMENSION view_dim = D3D11_SRV_DIMENSION_BUFFER;
		UINT flags = 0;

		auto srv_desc = D3D11_SHADER_RESOURCE_VIEW_DESC();
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.ViewDimension = view_dim;
		srv_desc.Buffer.FirstElement = desc.m_start_and_count.first;
		srv_desc.Buffer.NumElements = desc.m_start_and_count.second;
		
		// Handle RAW BUFFER (BufferEx) some other time


		m_dev->get_device()->CreateShaderResourceView(
			(ID3D11Resource*)buffer->m_internal_resource.Get(),
			&srv_desc,
			buffer->m_srv.GetAddressOf());

		//assert(false && "Buffer Shader Access view is not supported right now");
	}
	
	if (d3d_desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		D3D11_UAV_DIMENSION view_dim = D3D11_UAV_DIMENSION_BUFFER;
		UINT flags = 0;

		switch (desc.m_type)
		{
		case BufferType::eRaw:
			flags = D3D11_BUFFER_UAV_FLAG_RAW;
			break;
		case BufferType::eAppendConsume:	// AppendConsume Structured Buffer
			flags = D3D11_BUFFER_UAV_FLAG_APPEND;
			flags |= D3D11_BUFFER_UAV_FLAG_COUNTER;		// enable access to Increment/DecrementCounter
			break;


		}

		auto uav_desc = CD3D11_UNORDERED_ACCESS_VIEW_DESC
		(
			(ID3D11Buffer*)buffer->m_internal_resource.Get(),
			DXGI_FORMAT_UNKNOWN, desc.m_start_and_count.first, desc.m_start_and_count.second, flags
		);

		// spec requirement https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_buffer_uav_flag
		if ((flags & D3D11_BUFFER_UAV_FLAG_COUNTER) == D3D11_BUFFER_UAV_FLAG_COUNTER)
			assert(uav_desc.Format == DXGI_FORMAT_UNKNOWN);

		m_dev->get_device()->CreateUnorderedAccessView(
			(ID3D11Resource*)buffer->m_internal_resource.Get(),
			&uav_desc,
			buffer->m_uav.GetAddressOf());

		//assert(false && "Buffer Unordered Access View is not supported right now");
	}
}

void GfxDevice::create_texture(const TextureDesc& desc, GPUTexture* texture, std::optional<SubresourceData> subres)
{
	auto d3d_desc = desc.m_desc;

	//texture->m_desc.m_type = desc.m_type;

	// Grab misc. data
	bool is_array = d3d_desc.ArraySize > 1 ? true : false;
	bool ms_on = d3d_desc.SampleDesc.Count > 1 ? true : false;
	bool is_cube = d3d_desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE ? true : false;
	bool misc_gen_mips = d3d_desc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS ? true : false;

	// auto gen using GenerateMips requires texture to be written to (other subres)
	// we automatically append this incase user forgets
	if (misc_gen_mips)
		d3d_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

	if (ms_on && d3d_desc.MipLevels != 1)
		assert(false);		// https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_texture2d_desc MipLevels = 1 required for MS

	// check max multisample support
	UINT max_sample_quality_levels = 0;
	if (d3d_desc.SampleDesc.Count > 1)
	{
		m_dev->get_device()->CheckMultisampleQualityLevels(d3d_desc.Format, d3d_desc.SampleDesc.Count, &max_sample_quality_levels);
		d3d_desc.SampleDesc.Quality = (std::min)(d3d_desc.SampleDesc.Quality, max_sample_quality_levels - 1);
		std::cout << "Sample Quality: " << d3d_desc.SampleDesc.Quality << "\n";
	}

	// Create texture
	switch (desc.m_type)
	{
	case TextureType::e1D:
		assert(false && "Texture1D is not supported right now");
		break;
	case TextureType::e2D:
	{
		/*
			Using Generate Mips Misc Flag disallows using initial data
			https://stackoverflow.com/questions/53569263/directx-11-id3ddevicecreatetexture2d-with-initial-data-fail
			We will load it at top-level manually through context
		*/
		HRCHECK(m_dev->get_device()->CreateTexture2D(
			&d3d_desc,
			!misc_gen_mips && subres ? &subres->m_subres : nullptr,
			(ID3D11Texture2D**)texture->m_internal_resource.ReleaseAndGetAddressOf()));
		break;
	}
	case TextureType::e3D:
		assert(false && "Texture3D is not supported right now");
		break;
	default:
		assert(false);
		break;
	}

	texture->m_type = desc.m_type;
	//texture->m_desc.m_desc = d3d_desc;

	// Create views
	if (d3d_desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		// Find dimension
		D3D11_SRV_DIMENSION view_dim = D3D11_SRV_DIMENSION_UNKNOWN;
		switch (desc.m_type)
		{
		case TextureType::e1D:
			if (is_array)
				view_dim = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
			else
				view_dim = D3D11_SRV_DIMENSION_TEXTURE1D;
			break;
		case TextureType::e2D:
		{
			if (is_array)
			{
				if (is_cube)
				{
					view_dim = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
					break;
				}

				if (ms_on)
					view_dim = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
				else
					view_dim = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			}
			else
			{
				if (is_cube)
				{
					view_dim = D3D11_SRV_DIMENSION_TEXTURECUBE;
					break;
				}

				if (ms_on)
					view_dim = D3D11_SRV_DIMENSION_TEXTURE2DMS;
				else
					view_dim = D3D11_SRV_DIMENSION_TEXTURE2D;
			}
			break;
		}
		case TextureType::e3D:
			view_dim = D3D11_SRV_DIMENSION_TEXTURE3D;
			break;
		default:
			assert(false);
			break;
		}
		if (view_dim == D3D11_SRV_DIMENSION_UNKNOWN)
			assert(false);

		// Create desc
		D3D11_SHADER_RESOURCE_VIEW_DESC v_desc{};
		switch (desc.m_type)
		{
		case TextureType::e1D:
			assert(false && "SRV for Texture 1D is currently not supported");
			break;
		case TextureType::e2D:
			v_desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(
				(ID3D11Texture2D*)texture->m_internal_resource.Get(),
				view_dim,
				DXGI_FORMAT_UNKNOWN,	// setting to unknown --> auto resolves to the underlying texture format
				0,						// most detailed mip idx
				-1,						// max mips down to the least detailed
				0,						// first array slice	
				-1);					// array size (auto calc from tex)

			// Depth-stencil as read (read only depth part)
			if (v_desc.Format == DXGI_FORMAT_R32_TYPELESS)
				v_desc.Format = DXGI_FORMAT_R32_FLOAT;
			else if (v_desc.Format == DXGI_FORMAT_R32G8X24_TYPELESS)
				v_desc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
			else if (v_desc.Format == DXGI_FORMAT_R24G8_TYPELESS)
				v_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			else if (v_desc.Format == DXGI_FORMAT_R16_TYPELESS)
				v_desc.Format = DXGI_FORMAT_R16_UNORM;


			break;
		case TextureType::e3D:
			assert(false && "SRV for Texture 3D is currently not supported");
			break;
		default:
			assert(false);
			break;
		}

		// Create view
		HRCHECK(m_dev->get_device()->CreateShaderResourceView(
			(ID3D11Resource*)texture->m_internal_resource.Get(),
			&v_desc,
			texture->m_srv.ReleaseAndGetAddressOf()
		));

		// Auto-gen mips
		if (misc_gen_mips)
		{
			// Copy texture to top level first (initialization through CreateTexture2D is disabled when MISC_GEN_MIPS is on)

			/*
				We dont need to make staging resource because UpdateSubresource does that for us!
				https://stackoverflow.com/questions/50396189/d3d11-usage-staging-what-kind-of-gpu-cpu-memory-is-used
			*/
			m_dev->get_context()->UpdateSubresource((ID3D11Texture2D*)texture->m_internal_resource.Get(),
				0, nullptr, subres->m_subres.pSysMem, subres->m_subres.SysMemPitch, 0);

			assert(d3d_desc.BindFlags & D3D11_BIND_RENDER_TARGET);
			m_dev->get_context()->GenerateMips(texture->m_srv.Get());
		}

	}

	if (d3d_desc.BindFlags & D3D11_BIND_RENDER_TARGET)
	{
		// Find dimension
		D3D11_RTV_DIMENSION view_dim = D3D11_RTV_DIMENSION_UNKNOWN;
		D3D11_RENDER_TARGET_VIEW_DESC v_desc{};
		switch (desc.m_type)
		{
		case TextureType::e1D:
			if (is_array)
				view_dim = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
			else
				view_dim = D3D11_RTV_DIMENSION_TEXTURE1D;
			break;
		case TextureType::e2D:
			if (is_array)
			{
				if (ms_on)
					view_dim = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
				else
				{
					view_dim = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				}
			}
			else
			{
				if (ms_on)
					view_dim = D3D11_RTV_DIMENSION_TEXTURE2DMS;
				else
					view_dim = D3D11_RTV_DIMENSION_TEXTURE2D;
			}
			break;
		case TextureType::e3D:
			view_dim = D3D11_RTV_DIMENSION_TEXTURE3D;
			break;
		default:
			assert(false);
			break;
		}
		if (view_dim == D3D11_RTV_DIMENSION_UNKNOWN)
			assert(false);

		// Create desc
		switch (desc.m_type)
		{
		case TextureType::e1D:
			assert(false && "RTV for Texture 1D is currently not supported");
			break;
		case TextureType::e2D:

			if (view_dim == D3D11_RTV_DIMENSION_TEXTURE2D)
			{
				v_desc = CD3D11_RENDER_TARGET_VIEW_DESC(
					(ID3D11Texture2D*)texture->m_internal_resource.Get(),
					view_dim,
					DXGI_FORMAT_UNKNOWN,
					0,
					0,
					-1);	// array size (auto calc from tex)
			}
			else if (view_dim == D3D11_RTV_DIMENSION_TEXTURE2DARRAY)
			{
				v_desc = CD3D11_RENDER_TARGET_VIEW_DESC(
					(ID3D11Texture2D*)texture->m_internal_resource.Get(),
					view_dim,
					DXGI_FORMAT_UNKNOWN,
					0,
					0,						// start at subres idx 0
					d3d_desc.ArraySize);	// array size
			}
			break;
		case TextureType::e3D:
			assert(false && "RTV for Texture 3D is currently not supported");
			break;
		default:
			assert(false);
			break;
		}

		// Create view
		HRCHECK(m_dev->get_device()->CreateRenderTargetView(
			(ID3D11Resource*)texture->m_internal_resource.Get(),
			&v_desc,
			texture->m_rtv.ReleaseAndGetAddressOf()
		));
	}

	if (d3d_desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		// Find dimension
		D3D11_UAV_DIMENSION view_dim = D3D11_UAV_DIMENSION_UNKNOWN;
		D3D11_UNORDERED_ACCESS_VIEW_DESC v_desc{};
		switch (desc.m_type)
		{
		case TextureType::e1D:
			if (is_array)
				view_dim = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
			else
				view_dim = D3D11_UAV_DIMENSION_TEXTURE1D;
			break;
		case TextureType::e2D:
			if (is_array)
			{
				view_dim = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			}
			else
			{
				view_dim = D3D11_UAV_DIMENSION_TEXTURE2D;
			}
			break;
		case TextureType::e3D:
			view_dim = D3D11_UAV_DIMENSION_TEXTURE3D;
			break;
		default:
			assert(false);
			break;
		}
		if (view_dim == D3D11_UAV_DIMENSION_UNKNOWN)
			assert(false);


		// Create desc
		switch (desc.m_type)
		{
		case TextureType::e1D:
			assert(false && "UAV for Texture 1D is currently not supported");
			break;
		case TextureType::e2D:

			if (view_dim == D3D11_UAV_DIMENSION_TEXTURE2D)
			{
				v_desc = CD3D11_UNORDERED_ACCESS_VIEW_DESC(
					(ID3D11Texture2D*)texture->m_internal_resource.Get(),
					view_dim,
					DXGI_FORMAT_UNKNOWN,
					0,
					0,
					-1);	// array size (auto calc from tex)
			}
			else if (view_dim == D3D11_UAV_DIMENSION_TEXTURE2DARRAY)
			{
				v_desc = CD3D11_UNORDERED_ACCESS_VIEW_DESC(
					(ID3D11Texture2D*)texture->m_internal_resource.Get(),
					view_dim,
					DXGI_FORMAT_UNKNOWN,
					0,
					0,						// start at subres idx 0
					d3d_desc.ArraySize);	// array size
			}
			break;
		case TextureType::e3D:
			assert(false && "UAV for Texture 3D is currently not supported");
			break;
		default:
			assert(false);
			break;
		}

		// Create view
		HRCHECK(m_dev->get_device()->CreateUnorderedAccessView(
			(ID3D11Resource*)texture->m_internal_resource.Get(),
			&v_desc,
			texture->m_uav.ReleaseAndGetAddressOf()
		));
	}

	if (d3d_desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
	{
		// Find dimension
		D3D11_DSV_DIMENSION view_dim = D3D11_DSV_DIMENSION_UNKNOWN;
		switch (desc.m_type)
		{
		case TextureType::e1D:
			if (is_array)
				view_dim = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
			else
				view_dim = D3D11_DSV_DIMENSION_TEXTURE1D;
			break;
		case TextureType::e2D:
			if (is_array)
			{
				if (ms_on)
					view_dim = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
				else
					view_dim = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			}
			else
			{
				if (ms_on)
					view_dim = D3D11_DSV_DIMENSION_TEXTURE2DMS;
				else
					view_dim = D3D11_DSV_DIMENSION_TEXTURE2D;
			}
			break;
		case TextureType::e3D:
			assert(false);
			break;
		default:
			assert(false);
			break;
		}
		if (view_dim == D3D11_DSV_DIMENSION_UNKNOWN)
			assert(false);

		// Create desc
		D3D11_DEPTH_STENCIL_VIEW_DESC v_desc{};
		switch (desc.m_type)
		{
		case TextureType::e1D:
			assert(false && "RTV for Texture 1D is currently not supported");
			break;
		case TextureType::e2D:
			v_desc = CD3D11_DEPTH_STENCIL_VIEW_DESC(
				(ID3D11Texture2D*)texture->m_internal_resource.Get(),
				view_dim,
				DXGI_FORMAT_UNKNOWN,
				0,
				0,
				-1);	// array size (auto calc from tex)

			// Interpret depth stencil 
			if (v_desc.Format == DXGI_FORMAT_R32_TYPELESS)
				v_desc.Format = DXGI_FORMAT_D32_FLOAT;
			else if (v_desc.Format == DXGI_FORMAT_R32G8X24_TYPELESS)
				v_desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			else if (v_desc.Format == DXGI_FORMAT_R24G8_TYPELESS)
				v_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			else if (v_desc.Format == DXGI_FORMAT_R16_TYPELESS)
				v_desc.Format = DXGI_FORMAT_D16_UNORM;

			break;
		case TextureType::e3D:
			assert(false && "RTV for Texture 3D is currently not supported");
			break;
		default:
			assert(false);
			break;
		}

		// Create view
		HRCHECK(m_dev->get_device()->CreateDepthStencilView(
			(ID3D11Resource*)texture->m_internal_resource.Get(),
			&v_desc,
			texture->m_dsv.ReleaseAndGetAddressOf()
		));
	}

}

void GfxDevice::create_sampler(const SamplerDesc& desc, Sampler* sampler)
{
	HRCHECK(m_dev->get_device()->CreateSamplerState(&desc.m_sampler_desc,
		(ID3D11SamplerState**)sampler->m_internal_resource.ReleaseAndGetAddressOf()));
}

void GfxDevice::create_shader(ShaderStage stage, const ShaderBytecode& bytecode, Shader* shader)
{
	if (!bytecode.code)
	{
		//assert(false);
		return;
	}

	switch (stage)
	{
	case ShaderStage::eVertex:
		HRCHECK(m_dev->get_device()->CreateVertexShader(bytecode.code->data(), bytecode.code->size(), nullptr, (ID3D11VertexShader**)shader->m_internal_resource.ReleaseAndGetAddressOf()));
		shader->m_stage = ShaderStage::eVertex;
		shader->m_blob = bytecode;
		break;
	case ShaderStage::ePixel:
		HRCHECK(m_dev->get_device()->CreatePixelShader(bytecode.code->data(), bytecode.code->size(), nullptr, (ID3D11PixelShader**)shader->m_internal_resource.ReleaseAndGetAddressOf()));
		shader->m_stage = ShaderStage::ePixel;
		break;
	case ShaderStage::eHull:
		HRCHECK(m_dev->get_device()->CreateHullShader(bytecode.code->data(), bytecode.code->size(), nullptr, (ID3D11HullShader**)shader->m_internal_resource.ReleaseAndGetAddressOf()));
		shader->m_stage = ShaderStage::eHull;
		break;
	case ShaderStage::eDomain:
		HRCHECK(m_dev->get_device()->CreateDomainShader(bytecode.code->data(), bytecode.code->size(), nullptr, (ID3D11DomainShader**)shader->m_internal_resource.ReleaseAndGetAddressOf()));
		shader->m_stage = ShaderStage::eDomain;
		break;
	case ShaderStage::eGeometry:
		HRCHECK(m_dev->get_device()->CreateGeometryShader(bytecode.code->data(), bytecode.code->size(), nullptr, (ID3D11GeometryShader**)shader->m_internal_resource.ReleaseAndGetAddressOf()));
		shader->m_stage = ShaderStage::eGeometry;
		break;
	case ShaderStage::eCompute:
		HRCHECK(m_dev->get_device()->CreateComputeShader(bytecode.code->data(), bytecode.code->size(), nullptr, (ID3D11ComputeShader**)shader->m_internal_resource.ReleaseAndGetAddressOf()));
		shader->m_stage = ShaderStage::eCompute;
		break;
	}

	shader->m_blob = bytecode;
}

void GfxDevice::create_renderpass(const RenderPassDesc& desc, RenderPass* RenderPass)
{
	//RenderPass->m_depth_stencil_resolve_target = nullptr;
	//RenderPass->m_depth_stencil_target = nullptr;
	RenderPass->m_resolve_targets.clear();
	RenderPass->m_targets.clear();

	bool render_targets_exist = true;
	if (desc.m_targets.size() == 0)
		render_targets_exist = false;


	// Sanitize (verify sample counts)
	if (render_targets_exist)
	{
		const auto& tex = m_textures.look_up(std::get<TextureHandle>(desc.m_targets[0]).hdl);
		assert(tex->m_type == TextureType::e2D);
		D3D11_TEXTURE2D_DESC d3d_desc_0{};
		((ID3D11Texture2D*)tex->m_internal_resource.Get())->GetDesc(&d3d_desc_0);
		/*
			https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-omsetrendertargets
			If render targets use multisample anti-aliasing, all bound render targets and depth buffer
			must be the same form of multisample resource (that is, the sample counts must be the same).
		*/

		// if multisamp on, sanitize
		if (d3d_desc_0.SampleDesc.Count > 1)
		{
			assert(desc.m_targets.size() == desc.m_resolve_targets.size()); // check that resolve targets exists for all render targets

			// check so that all bound rts and ds have ethe same sample counts (as per above specs)
			UINT samp_count = d3d_desc_0.SampleDesc.Count;
			for (int i = 0; i < desc.m_targets.size(); ++i)
			{
				const auto& other_tex = m_textures.look_up(std::get<TextureHandle>(desc.m_targets[i]).hdl);
				assert(other_tex->m_type == TextureType::e2D);
				D3D11_TEXTURE2D_DESC d3d_desc_n{};
				((ID3D11Texture2D*)other_tex->m_internal_resource.Get())->GetDesc(&d3d_desc_n);

				assert(d3d_desc_n.SampleDesc.Count == samp_count);
			}

			// Verify with depth stencil target if exists
			if (desc.m_depth_stencil_target.hdl != 0)
			{
				assert(m_textures.look_up(desc.m_depth_stencil_target.hdl)->m_type == TextureType::e2D);
				D3D11_TEXTURE2D_DESC ds_desc{};
				((ID3D11Texture2D*)m_textures.look_up(desc.m_depth_stencil_target.hdl)->m_internal_resource.Get())->GetDesc(&ds_desc);

				assert(ds_desc.SampleDesc.Count == samp_count);
			}

		}
	}

	if (desc.m_depth_stencil_target.hdl != 0)
	{
		// Add depth-stencil target
		RenderPass->m_depth_stencil_target = desc.m_depth_stencil_target.hdl != 0 ? desc.m_depth_stencil_target : TextureHandle{ 0 };

		// dsv dont necessarily have to be resolved
		// but do resolve it if a resolve target is supplied
		if (desc.m_depth_stencil_target_resolve.hdl != 0)
			RenderPass->m_depth_stencil_resolve_target = desc.m_depth_stencil_target_resolve;
	}


	// Add render targets
	RenderPass->m_targets.reserve(desc.m_targets.size());
	for (int i = 0; i < desc.m_targets.size(); ++i)
	{
		const auto& other_tex = m_textures.look_up(std::get<TextureHandle>(desc.m_targets[i]).hdl);
		assert(other_tex->m_type == TextureType::e2D);
		D3D11_TEXTURE2D_DESC d3d_desc_n{};
		((ID3D11Texture2D*)other_tex->m_internal_resource.Get())->GetDesc(&d3d_desc_n);

		if (std::get<TextureHandle>(desc.m_targets[i]).hdl != 0)
		{
			RenderPass->m_targets.push_back({
				std::get<TextureHandle>(desc.m_targets[i]),
				std::get<RenderTextureClear>(desc.m_targets[i]),
				d3d_desc_n.Format,
				d3d_desc_n.SampleDesc
				});
		}
	}
	RenderPass->m_resolve_targets = desc.m_resolve_targets;

	RenderPass->m_is_registered = true;
}

void GfxDevice::create_pipeline(const PipelineDesc& desc, GraphicsPipeline* pipeline)
{
	auto& dev = m_dev->get_device();

	// create rasterizer state (default state if user dont supply)
	HRCHECK(dev->CreateRasterizerState1(&desc.m_rasterizer_desc.m_rasterizer_desc,
		(ID3D11RasterizerState1**)pipeline->m_rasterizer.m_internal_resource.ReleaseAndGetAddressOf()));

	// create blend state (default state if user dont supply)
	HRCHECK(dev->CreateBlendState1(&desc.m_blend_desc.m_blend_desc,
		(ID3D11BlendState1**)pipeline->m_blend.m_internal_resource.ReleaseAndGetAddressOf()));

	HRCHECK(dev->CreateDepthStencilState(&desc.m_depth_stencil_desc.m_depth_stencil_desc,
		(ID3D11DepthStencilState**)pipeline->m_depth_stencil.m_internal_resource.ReleaseAndGetAddressOf()));

	// Check shader validity
	auto vs = m_shaders.look_up(desc.m_vs.hdl);
	auto ps = m_shaders.look_up(desc.m_ps.hdl);

	assert(vs->get_stage() == ShaderStage::eVertex);
	assert(ps->get_stage() == ShaderStage::ePixel);

	if (desc.m_hs.has_value())
	{
		assert(desc.m_hs->hdl != 0);
		assert(m_shaders.look_up(desc.m_hs->hdl)->get_stage() == ShaderStage::eHull);
	}

	if (desc.m_gs.has_value())
	{
		assert(desc.m_gs->hdl != 0);
		assert(m_shaders.look_up(desc.m_gs->hdl)->get_stage() == ShaderStage::eGeometry);
	}

	if (desc.m_ds.has_value())
	{
		assert(desc.m_ds->hdl != 0);
		assert(m_shaders.look_up(desc.m_ds->hdl)->get_stage() == ShaderStage::eDomain);
	}


	// create input layout (duplicates may be created here, we will ignore this for simplicity)
	if (!desc.m_input_desc.m_input_descs.empty())
	{
		auto vs = m_shaders.look_up(desc.m_vs.hdl);

		HRCHECK(dev->CreateInputLayout(
			desc.m_input_desc.m_input_descs.data(),
			(UINT)desc.m_input_desc.m_input_descs.size(),
			vs->m_blob.code->data(),
			(UINT)vs->m_blob.code->size(),
			(ID3D11InputLayout**)pipeline->m_input_layout.m_internal_resource.ReleaseAndGetAddressOf()));
	}

	// add shaders
	pipeline->m_vs = desc.m_vs;
	pipeline->m_ps = desc.m_ps;


	pipeline->m_is_registered = true;
}



void GfxDevice::begin_pass(const RenderPass* RenderPass, DepthStencilClear ds_clear)
{
	if (!RenderPass->m_is_registered)
	{
		assert(false && "RenderPass is not registered!");
		return;
	}

	m_active_rp = RenderPass;
	m_inside_pass = true;

	auto& ctx = m_dev->get_context();
	auto& depth_tex = RenderPass->m_depth_stencil_target;

	// clear RenderPass and get render targets
	ID3D11RenderTargetView* rtvs[gfxconstants::MAX_RENDER_TARGETS] = {};
	for (int i = 0; i < RenderPass->m_targets.size(); ++i)
	{
		// get target
		auto& target = RenderPass->m_targets[i];
		if (std::get<TextureHandle>(target).hdl == 0)
			break;
		rtvs[i] = m_textures.look_up(std::get<TextureHandle>(target).hdl)->m_rtv.Get();

		// clear target
		ctx->ClearRenderTargetView(m_textures.look_up(std::get<TextureHandle>(target).hdl)->m_rtv.Get(), std::get<RenderTextureClear>(target).m_rgba.data());
	}

	ID3D11DepthStencilView* dsv = nullptr;
	if (depth_tex.hdl != 0)
	{
		dsv = m_textures.look_up(depth_tex.hdl)->m_dsv.Get();
		ctx->ClearDepthStencilView(dsv, ds_clear.m_clear_flags, ds_clear.m_depth, ds_clear.m_stencil);
	}

	if (m_raster_rw_range_this_pass > 0)
	{
		ctx->OMSetRenderTargetsAndUnorderedAccessViews(
			gfxconstants::MAX_RENDER_TARGETS, rtvs, dsv,
			0, gfxconstants::MAX_BOUND_UAVS, m_raster_uavs.data(), m_raster_uav_initial_counts.data());
	}
	else
	{
		ctx->OMSetRenderTargets(gfxconstants::MAX_RENDER_TARGETS, rtvs, dsv);
	}

}

void GfxDevice::bind_constant_buffer(UINT slot, ShaderStage stage, const GPUBuffer* buffer, UINT offset256s, UINT range256s)
{
	ID3D11Buffer* cbs[] = { (ID3D11Buffer*)buffer->m_internal_resource.Get() };
	auto& ctx = m_dev->get_context();

	UINT first_constant = offset256s * 16;
	UINT num_constants = range256s * 16;

	switch (stage)
	{
	case ShaderStage::eVertex:
		//ctx->VSSetConstantBuffers(slot, 1, cbs);
		ctx->VSSetConstantBuffers1(slot, 1, cbs, &first_constant, &num_constants);
		break;
	case ShaderStage::ePixel:
		//ctx->PSSetConstantBuffers(slot, 1, cbs);
		ctx->PSSetConstantBuffers1(slot, 1, cbs, &first_constant, &num_constants);
		break;
	case ShaderStage::eHull:
		//ctx->HSSetConstantBuffers(slot, 1, cbs);
		ctx->HSSetConstantBuffers1(slot, 1, cbs, &first_constant, &num_constants);
		break;
	case ShaderStage::eDomain:
		//ctx->DSSetConstantBuffers(slot, 1, cbs);
		ctx->DSSetConstantBuffers1(slot, 1, cbs, &first_constant, &num_constants);
		break;
	case ShaderStage::eGeometry:
		//ctx->GSSetConstantBuffers(slot, 1, cbs);
		ctx->GSSetConstantBuffers1(slot, 1, cbs, &first_constant, &num_constants);
		break;
	case ShaderStage::eCompute:
		//ctx->CSSetConstantBuffers(slot, 1, cbs);
		ctx->CSSetConstantBuffers1(slot, 1, cbs, &first_constant, &num_constants);
		break;
	}
}

void GfxDevice::bind_resource(UINT slot, ShaderStage stage, const GPUResource* resource)
{
	ID3D11ShaderResourceView* srvs[] = { resource ? resource->m_srv.Get() : nullptr };
	auto& ctx = m_dev->get_context();

	switch (stage)
	{
	case ShaderStage::eVertex:
		ctx->VSSetShaderResources(slot, 1, srvs);
		break;
	case ShaderStage::ePixel:
		ctx->PSSetShaderResources(slot, 1, srvs);
		break;
	case ShaderStage::eHull:
		ctx->HSSetShaderResources(slot, 1, srvs);
		break;
	case ShaderStage::eDomain:
		ctx->DSSetShaderResources(slot, 1, srvs);
		break;
	case ShaderStage::eGeometry:
		ctx->GSSetShaderResources(slot, 1, srvs);
		break;
	case ShaderStage::eCompute:
		ctx->CSSetShaderResources(slot, 1, srvs);
		break;
	}
}

void GfxDevice::bind_resource_rw(UINT slot, ShaderStage stage, const GPUResource* resource, UINT initial_count)
{
	//assert(m_inside_pass == true && "Resource RWs must be bound prior to begin_pass()");

	ID3D11UnorderedAccessView* uavs[] = { resource->m_uav.Get() };
	auto& ctx = m_dev->get_context();

	switch (stage)
	{
		// https://docs.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-1-features#use-uavs-at-every-pipeline-stage
		// 11.1, use UAVs at every pipeline stage
	case ShaderStage::eVertex:
	case ShaderStage::ePixel:
	case ShaderStage::eHull:
	case ShaderStage::eDomain:
	case ShaderStage::eGeometry:
	{
		m_raster_uavs[slot] = resource->m_uav.Get();
		m_raster_uav_initial_counts[slot] = initial_count;
		auto range = m_raster_rw_range_this_pass;
		// https://github.com/assimp/assimp/issues/2271 paranthesis solves DEFINE NOMINMAX
		m_raster_rw_range_this_pass = (std::max)(range, slot);
		break;
	}
	case ShaderStage::eCompute:
		ctx->CSSetUnorderedAccessViews(slot, 1, uavs, &initial_count);
		break;
	}

}

void GfxDevice::bind_sampler(UINT slot, ShaderStage stage, const Sampler* sampler)
{
	ID3D11SamplerState* samplers[] = { (ID3D11SamplerState*)sampler->m_internal_resource.Get() };
	auto& ctx = m_dev->get_context();

	switch (stage)
	{
	case ShaderStage::eVertex:
		ctx->VSSetSamplers(slot, 1, samplers);
		break;
	case ShaderStage::ePixel:
		ctx->PSSetSamplers(slot, 1, samplers);
		break;
	case ShaderStage::eHull:
		ctx->HSSetSamplers(slot, 1, samplers);
		break;
	case ShaderStage::eDomain:
		ctx->DSSetSamplers(slot, 1, samplers);
		break;
	case ShaderStage::eGeometry:
		ctx->GSSetSamplers(slot, 1, samplers);
		break;
	case ShaderStage::eCompute:
		ctx->CSSetSamplers(slot, 1, samplers);
		break;
	}
}

void GfxDevice::update_subresource(const GPUResource* dst, const SubresourceData& data, const D3D11_BOX& dst_box, UINT dst_subres_idx)
{
	m_dev->get_context()->UpdateSubresource((ID3D11Resource*)dst->m_internal_resource.Get(),
		dst_subres_idx, &dst_box, data.m_subres.pSysMem, data.m_subres.SysMemPitch, data.m_subres.SysMemSlicePitch);
}

void GfxDevice::map_copy(const GPUResource* dst, const SubresourceData& data, D3D11_MAP map_type, UINT dst_subres_idx)
{
	if (data.m_subres.pSysMem == nullptr || data.m_subres.SysMemPitch == 0)
		return;

	assert(data.m_subres.pSysMem != nullptr && data.m_subres.SysMemPitch != 0);
	auto& ctx = m_dev->get_context();
	D3D11_MAPPED_SUBRESOURCE mapped_subres{};
	HRCHECK(ctx->Map((ID3D11Resource*)dst->m_internal_resource.Get(), dst_subres_idx, map_type, 0, &mapped_subres));
	std::memcpy(mapped_subres.pData, data.m_subres.pSysMem, data.m_subres.SysMemPitch);		// not handling slice pitch for now
	ctx->Unmap((ID3D11Resource*)dst->m_internal_resource.Get(), dst_subres_idx);
}

void GfxDevice::bind_pipeline(const GraphicsPipeline* pipeline, std::array<FLOAT, 4> blend_factor, UINT stencil_ref)
{
	if (!pipeline->m_is_registered)
	{
		assert(false);
		return;
	}

	// Identical pipeline already bound
	// We do checking at a pipeline level, just to get comfy with DX12/VK with PSOs/Pipeline
	if (m_curr_pipeline == pipeline)
		return;

	m_curr_pipeline = pipeline;

	auto& ctx = m_dev->get_context();

	if (pipeline->m_input_layout.is_valid())
		ctx->IASetInputLayout((ID3D11InputLayout*)pipeline->m_input_layout.m_internal_resource.Get());
	else
		ctx->IASetInputLayout(nullptr);


	// bind shaders
	ctx->VSSetShader((ID3D11VertexShader*)m_shaders.look_up(pipeline->m_vs.hdl)->m_internal_resource.Get(), nullptr, 0);
	ctx->PSSetShader((ID3D11PixelShader*)m_shaders.look_up(pipeline->m_ps.hdl)->m_internal_resource.Get(), nullptr, 0);

	if (pipeline->m_gs.hdl != 0)
		ctx->GSSetShader((ID3D11GeometryShader*)m_shaders.look_up(pipeline->m_gs.hdl)->m_internal_resource.Get(), nullptr, 0);
	else
		ctx->GSSetShader(nullptr, nullptr, 0);

	if (pipeline->m_hs.hdl != 0)
		ctx->HSSetShader((ID3D11HullShader*)m_shaders.look_up(pipeline->m_hs.hdl)->m_internal_resource.Get(), nullptr, 0);
	else
		ctx->HSSetShader(nullptr, nullptr, 0);

	if (pipeline->m_ds.hdl != 0)
		ctx->DSSetShader((ID3D11DomainShader*)m_shaders.look_up(pipeline->m_ds.hdl)->m_internal_resource.Get(), nullptr, 0);
	else
		ctx->DSSetShader(nullptr, nullptr, 0);

	ctx->IASetPrimitiveTopology(pipeline->m_topology);
	ctx->RSSetState((ID3D11RasterizerState*)pipeline->m_rasterizer.m_internal_resource.Get());
	ctx->OMSetDepthStencilState((ID3D11DepthStencilState*)pipeline->m_depth_stencil.m_internal_resource.Get(), stencil_ref);
	ctx->OMSetBlendState((ID3D11BlendState*)pipeline->m_blend.m_internal_resource.Get(), blend_factor.data(), pipeline->m_sample_mask);
}


