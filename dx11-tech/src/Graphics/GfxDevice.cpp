#include "pch.h"
#include "Graphics/GfxDevice.h"

// Globals
namespace gfx
{ 
	GfxDevice* dev = nullptr;  
	GPUAnnotator* annotator = nullptr;
}

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
	m_backbuffer.m_internal_resource = m_dev->get_bb_texture();
	m_backbuffer.m_rtv = m_dev->get_bb_target();
	m_backbuffer.m_desc.m_type = TextureType::e2D;
	m_backbuffer.m_desc.m_render_target_clear = RenderTextureClear::black();
	m_dev->get_bb_texture()->GetDesc(&m_backbuffer.m_desc.m_desc);

	// Initialize annotator
	m_annotator = make_unique<GPUAnnotator>(m_dev->get_annotation());

	// Initialize profiler
	m_profiler = make_unique<GPUProfiler>(m_dev.get());
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
	if (m_profiler)
		m_profiler->frame_end();
}



void GfxDevice::compile_and_create_shader(ShaderStage stage, const std::filesystem::path& fname, Shader* shader)
{
	ShaderBytecode bc;
	compile_shader(stage, fname, &bc);
	create_shader(stage, bc, shader);
}

void GfxDevice::compile_shader(ShaderStage stage, const std::filesystem::path& fname, ShaderBytecode* bytecode)
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
			//assert(false);
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
	/*
		using the current solution with the map can lead to a dangling pointer!!
		all pipelines created must be kept alive or else recompilation will iterate over 
		pipelines that are potentially dead.
	*/
	if (!m_reloading_on)
		return;

	auto fname = name + std::string(".hlsl");		// append extension

	// grab all the pipeline states connected with the shader name
	auto it = m_loaded_pipelines.find(fname);
	if (it == m_loaded_pipelines.end())
	{
		assert(false);
	}
	auto& pipelines = it->second;

	const auto& sample_pipeline = pipelines[0];

	// get filenames to all shaders in the pipelines
	const auto& vs_name = sample_pipeline->m_vs.m_blob.fname;
	const auto& ps_name = sample_pipeline->m_ps.m_blob.fname;
	const auto& gs_name = sample_pipeline->m_gs.m_blob.fname;
	const auto& hs_name = sample_pipeline->m_hs.m_blob.fname;
	const auto& ds_name = sample_pipeline->m_ds.m_blob.fname;

	// compile shaders
	Shader vs, ps, gs, ds, hs;
	compile_and_create_shader(ShaderStage::eVertex, vs_name, &vs);
	compile_and_create_shader(ShaderStage::ePixel, ps_name, &ps);

	if (!gs_name.empty())
		compile_and_create_shader(ShaderStage::eGeometry, gs_name, &gs);

	if (!hs_name.empty())
		compile_and_create_shader(ShaderStage::eHull, hs_name, &hs);

	if (!ds_name.empty())
		compile_and_create_shader(ShaderStage::eDomain, ds_name, &ds);

	/*
		dangling pointer possible if Pipeline is destroyed outside.
		Ignoring for now
	*/
	for (auto& pipeline : pipelines)
	{
		if (vs.is_valid())
			pipeline->m_vs = vs;
		if (ps.is_valid())
			pipeline->m_ps = ps;
		pipeline->m_gs = gs;
		pipeline->m_hs = hs;
		pipeline->m_ds = ds;
	}
}

void GfxDevice::create_buffer(const BufferDesc& desc, GPUBuffer* buffer, std::optional<SubresourceData> subres)
{
	const auto& d3d_desc = desc.m_desc;

	buffer->m_desc.m_type = desc.m_type;

	HRCHECK(m_dev->get_device()->CreateBuffer(
		&d3d_desc,
		subres ? &subres->m_subres : nullptr,
		(ID3D11Buffer**)buffer->m_internal_resource.ReleaseAndGetAddressOf()));

	buffer->m_desc.m_desc = d3d_desc;

	// Create views
	if (d3d_desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		assert(false && "Buffer Shader Access view is not supported right now");
	}
	else if (d3d_desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		assert(false && "Buffer Unordered Access View is not supported right now");
	}
}

void GfxDevice::create_texture(const TextureDesc& desc, GPUTexture* texture, std::optional<SubresourceData> subres)
{
	auto d3d_desc = desc.m_desc;

	texture->m_desc.m_type = desc.m_type;

	// Grab misc. data
	bool is_array = d3d_desc.ArraySize > 1 ? true : false;
	bool ms_on = d3d_desc.SampleDesc.Count > 1 ? true : false;
	bool is_cube = d3d_desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE ? true : false;

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
		HRCHECK(m_dev->get_device()->CreateTexture2D(
			&d3d_desc,
			subres ? &subres->m_subres : nullptr,
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

	texture->m_desc.m_desc = d3d_desc;

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
				DXGI_FORMAT_UNKNOWN,	// auto resolves to the texture format
				0,						// most detailed mip idx
				-1,						// max mips down to the least detailed
				0,						// first array slice	
				-1);					// array size (auto calc from tex)
			
			// Depth-stencil as read (read only 32-bit depth part)
			if (v_desc.Format == DXGI_FORMAT_R32_TYPELESS)
				v_desc.Format = DXGI_FORMAT_R32_FLOAT;
			else if (v_desc.Format == DXGI_FORMAT_R32G8X24_TYPELESS)
				v_desc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
			else if (v_desc.Format == DXGI_FORMAT_R24G8_TYPELESS)
				v_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

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
		if (d3d_desc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
		{
			assert(d3d_desc.BindFlags & D3D11_BIND_RENDER_TARGET);
			m_dev->get_context()->GenerateMips(texture->m_srv.Get());
		}

	}
	
	if (d3d_desc.BindFlags & D3D11_BIND_RENDER_TARGET)
	{
		// Find dimension
		D3D11_RTV_DIMENSION view_dim = D3D11_RTV_DIMENSION_UNKNOWN;
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
					view_dim = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
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
		D3D11_RENDER_TARGET_VIEW_DESC v_desc{};
		switch (desc.m_type)
		{
		case TextureType::e1D:
			assert(false && "RTV for Texture 1D is currently not supported");
			break;
		case TextureType::e2D:
			v_desc = CD3D11_RENDER_TARGET_VIEW_DESC(
				(ID3D11Texture2D*)texture->m_internal_resource.Get(),
				view_dim,
				DXGI_FORMAT_UNKNOWN,
				0,
				0,
				-1);	// array size (auto calc from tex)
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
		assert(false && "Texture Unordered Access View is not supported right now");

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

void GfxDevice::create_framebuffer(const FramebufferDesc& desc, Framebuffer* framebuffer)
{
	bool render_targets_exist = true;
	if (desc.m_targets.size() == 0)
		render_targets_exist = false;


	if (render_targets_exist && desc.m_depth_stencil_target)
	{
		const auto& d3d_desc_0 = desc.m_targets[0];
		/*
			https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-omsetrendertargets
			If render targets use multisample anti-aliasing, all bound render targets and depth buffer
			must be the same form of multisample resource (that is, the sample counts must be the same).
		*/

		// if multisamp on, sanitize
		if (d3d_desc_0->m_desc.m_desc.SampleDesc.Count > 1)
		{
			assert(desc.m_targets.size() == desc.m_resolve_targets.size()); // check that resolve targets exists for all render targets

			// check so that all bound rts and ds have ethe same sample counts (as per above specs)
			UINT samp_count = d3d_desc_0->m_desc.m_desc.SampleDesc.Count;
			for (int i = 0; i < desc.m_targets.size(); ++i)
				assert(desc.m_targets[i]->m_desc.m_desc.SampleDesc.Count == samp_count);
			assert(desc.m_depth_stencil_target->m_desc.m_desc.SampleDesc.Count == samp_count);
		}

	}

	// Add depth-stencil target
	framebuffer->m_depth_stencil_target = desc.m_depth_stencil_target ? desc.m_depth_stencil_target : nullptr;
	// dsv dont necessarily have to be resolved
	// but do resolve it if a resolve target is supplied
	if (desc.m_depth_stencil_target_resolve)
		framebuffer->m_depth_stencil_resolve_target = desc.m_depth_stencil_target_resolve;

	// Add render targets
	framebuffer->m_targets.reserve(desc.m_targets.size());
	for (int i = 0; i < desc.m_targets.size(); ++i)
	{
		if (desc.m_targets[i])
			framebuffer->m_targets.push_back(desc.m_targets[i]);
	}
	framebuffer->m_resolve_targets = desc.m_resolve_targets;

	framebuffer->m_is_registered = true;
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

	// create input layout (duplicates may be created here, we will ignore this for simplicity)
	if (!desc.m_input_desc.m_input_descs.empty())
	{
		HRCHECK(dev->CreateInputLayout(
			desc.m_input_desc.m_input_descs.data(),
			(UINT)desc.m_input_desc.m_input_descs.size(),
			desc.m_vs.m_blob.code->data(),
			(UINT)desc.m_vs.m_blob.code->size(),
			(ID3D11InputLayout**)pipeline->m_input_layout.m_internal_resource.ReleaseAndGetAddressOf()));
	}

	auto load_to_cache = [&](const std::string& fname)
	{
		auto it = m_loaded_pipelines.find(fname);
		if (it != m_loaded_pipelines.end())
			it->second.push_back(pipeline);
		else
			m_loaded_pipelines.insert({ fname, { pipeline } });
	};

	// add shaders and add pipeline to lookup table
	pipeline->m_vs = desc.m_vs;
	pipeline->m_ps = desc.m_ps;

	if (m_reloading_on)
	{
		load_to_cache(desc.m_vs.m_blob.fname);
		load_to_cache(desc.m_ps.m_blob.fname);
	}


	if (desc.m_gs.has_value())
	{
		pipeline->m_gs = *desc.m_gs;
		if (m_reloading_on)
			load_to_cache(desc.m_gs->m_blob.fname);
	
	}
	if (desc.m_hs.has_value())
	{
		pipeline->m_hs = *desc.m_hs;
		if (m_reloading_on)
			load_to_cache(desc.m_hs->m_blob.fname);

	}
	if (desc.m_ds.has_value())
	{
		pipeline->m_ds = *desc.m_ds;
		if (m_reloading_on)
			load_to_cache(desc.m_ds->m_blob.fname);
	}

	pipeline->m_is_registered = true;
}


void GfxDevice::set_name(const GPUType* device_child, const std::string& name)
{
	HRCHECK(m_dev->get_context()->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.data()));
}

void GfxDevice::dispatch(UINT blocks_x, UINT blocks_y, UINT blocks_z)
{
	auto& ctx = m_dev->get_context();

	ctx->Dispatch(blocks_x, blocks_y, blocks_z);

	// https://on-demand.gputechconf.com/gtc/2010/presentations/S12312-DirectCompute-Pre-Conference-Tutorial.pdf
	// How the hell do you know when it is safe to Unbind UAVs from Compute Shader???
	// Ans: Check GP Discord, DirectX section, answer from jwki
	// No need to sync with Fence
	// https://stackoverflow.com/questions/55005420/how-to-do-a-blocking-wait-for-a-compute-shader-with-direct3d11
	ctx->CSSetUnorderedAccessViews(0, gfxconstants::MAX_CS_UAV, (ID3D11UnorderedAccessView* const*)gfxconstants::NULL_RESOURCE, (const UINT*)gfxconstants::NULL_RESOURCE);
}

void GfxDevice::begin_pass(const Framebuffer* framebuffer, DepthStencilClear ds_clear)
{
	if (!framebuffer->m_is_registered)
	{
		assert(false && "Framebuffer is not registered!");
		return;
	}
	
	m_active_framebuffer = framebuffer;
	m_inside_pass = true;

	auto& ctx = m_dev->get_context();
	auto& depth_tex = framebuffer->m_depth_stencil_target;

	// clear framebuffer and get render targets
	ID3D11RenderTargetView* rtvs[gfxconstants::MAX_RENDER_TARGETS] = {};
	for (int i = 0; i < framebuffer->m_targets.size(); ++i)
	{
		// get target
		auto& target = framebuffer->m_targets[i];
		if (!target)
			break;
		rtvs[i] = target->m_rtv.Get();

		// clear target
		ctx->ClearRenderTargetView(target->m_rtv.Get(), target->m_desc.m_render_target_clear.m_rgba.data());
	}

	ID3D11DepthStencilView* dsv = nullptr;
	if (depth_tex)
	{
		dsv = depth_tex->m_dsv.Get();
		ctx->ClearDepthStencilView(dsv, ds_clear.m_clear_flags, ds_clear.m_depth, ds_clear.m_stencil);
	}

	if (m_raster_rw_range_this_pass > 0)
	{
		ctx->OMSetRenderTargetsAndUnorderedAccessViews(
			gfxconstants::MAX_RENDER_TARGETS, rtvs, dsv,
			0, gfxconstants::MAX_RASTER_UAVS, m_raster_uavs.data(), m_raster_uav_initial_counts.data());
	}
	else
	{
		ctx->OMSetRenderTargets(gfxconstants::MAX_RENDER_TARGETS, rtvs, dsv);
	}

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
			0, gfxconstants::MAX_RASTER_UAVS, (ID3D11UnorderedAccessView* const*)gfxconstants::NULL_RESOURCE, nullptr);
		m_raster_rw_range_this_pass = 0;
	}
	else
	{
		ctx->OMSetRenderTargets(gfxconstants::MAX_RENDER_TARGETS, (ID3D11RenderTargetView* const*)gfxconstants::NULL_RESOURCE, nullptr);
	}


	// resolve any ms targets if any
	if (!m_active_framebuffer->m_resolve_targets.empty())
	{
		for (int i = 0; i < m_active_framebuffer->m_targets.size(); ++i)
		{
			auto src = (ID3D11Texture2D*)m_active_framebuffer->m_targets[i]->m_internal_resource.Get();
			auto dst = (ID3D11Texture2D*)m_active_framebuffer->m_resolve_targets[i]->m_internal_resource.Get();
			auto format = m_active_framebuffer->m_targets[i]->m_desc.m_desc.Format;
			ctx->ResolveSubresource(dst, 0, src, 0, format);
		}
	}

	// TO-DO: resolve depth target using compute shader
	// https://wickedengine.net/2016/11/13/how-to-resolve-an-msaa-depthbuffer/#comments

	m_active_framebuffer = nullptr;
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

void GfxDevice::bind_constant_buffer(UINT slot, ShaderStage stage, const GPUBuffer* buffer, UINT offset_256s, UINT range_256s)
{
	ID3D11Buffer* cbs[] = { (ID3D11Buffer*)buffer->m_internal_resource.Get() };
	auto& ctx = m_dev->get_context();

	UINT first_constant = offset_256s * 16;
	UINT num_constants = range_256s * 16;

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
	ID3D11ShaderResourceView* srvs[] = { resource->m_srv.Get() };
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
	assert(m_inside_pass == true && "Resource RWs must be bound prior to begin_pass()");

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

	auto& ctx = m_dev->get_context();

	if (pipeline->m_input_layout.is_valid())
		ctx->IASetInputLayout((ID3D11InputLayout*)pipeline->m_input_layout.m_internal_resource.Get());
	else
		ctx->IASetInputLayout(nullptr);

	// bind shaders
	ctx->VSSetShader((ID3D11VertexShader*)pipeline->m_vs.m_internal_resource.Get(), nullptr, 0);
	ctx->PSSetShader((ID3D11PixelShader*)pipeline->m_ps.m_internal_resource.Get(), nullptr, 0);

	if (pipeline->m_gs.is_valid())
		ctx->GSSetShader((ID3D11GeometryShader*)pipeline->m_gs.m_internal_resource.Get(), nullptr, 0);
	else
		ctx->GSSetShader(nullptr, nullptr, 0);

	if (pipeline->m_hs.is_valid())
		ctx->HSSetShader((ID3D11HullShader*)pipeline->m_hs.m_internal_resource.Get(), nullptr, 0);
	else
		ctx->HSSetShader(nullptr, nullptr, 0);

	if (pipeline->m_ds.is_valid())
		ctx->DSSetShader((ID3D11DomainShader*)pipeline->m_ds.m_internal_resource.Get(), nullptr, 0);
	else
		ctx->DSSetShader(nullptr, nullptr, 0);

	ctx->IASetPrimitiveTopology(pipeline->m_topology);
	ctx->RSSetState((ID3D11RasterizerState*)pipeline->m_rasterizer.m_internal_resource.Get());
	ctx->OMSetDepthStencilState((ID3D11DepthStencilState*)pipeline->m_depth_stencil.m_internal_resource.Get(), stencil_ref);
	ctx->OMSetBlendState((ID3D11BlendState*)pipeline->m_blend.m_internal_resource.Get(), blend_factor.data(), pipeline->m_sample_mask);
}

void GfxDevice::bind_vertex_buffers(UINT start_slot, UINT count, const GPUBuffer* buffers, UINT* strides, UINT* offsets)
{
	assert(count < 31);	// slot 31 is reserved for instancing
	ID3D11Buffer* vbs[gfxconstants::MAX_INPUT_SLOTS] = {};
	for (UINT i = 0; i < count; ++i)
	{
		vbs[i] = (ID3D11Buffer*)buffers[i].m_internal_resource.Get();
	}
	m_dev->get_context()->IASetVertexBuffers(start_slot, count, vbs,
		strides ? strides : (UINT*)gfxconstants::NULL_RESOURCE,
		offsets ? offsets : (UINT*)gfxconstants::NULL_RESOURCE);
}

void GfxDevice::bind_index_buffer(const GPUBuffer* buffer, DXGI_FORMAT format, UINT offset)
{
	m_dev->get_context()->IASetIndexBuffer((ID3D11Buffer*)buffer->m_internal_resource.Get(), format, offset);
}




GPUTexture* GfxDevice::get_backbuffer()
{
	return &m_backbuffer;
}

GPUProfiler* GfxDevice::get_profiler()
{
	return m_profiler.get();
}

GPUAnnotator* GfxDevice::get_annotator()
{
	return m_annotator.get();
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

