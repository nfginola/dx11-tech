#include "pch.h"
#include "Graphics/GfxDevice.h"
#include "Graphics/GfxCommon.h"

static GfxDevice* s_gfx_device = nullptr;

namespace gfxconstants
{
	// For unbinding state
	const void* const NULL_RESOURCE[gfxconstants::MAX_SHADER_INPUT_RESOURCE_SLOTS] = {};
}

void GfxDevice::initialize(unique_ptr<DXDevice> dev)
{
	if (!s_gfx_device)
		s_gfx_device = new GfxDevice(std::move(dev));
	else
		assert(false);	// dont try initializing multiple times..
}

void GfxDevice::shutdown()
{
	if (s_gfx_device)
		delete s_gfx_device;
}

GfxDevice* GfxDevice::get()
{
	return s_gfx_device;
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


}

GfxDevice::~GfxDevice()
{

}

void GfxDevice::frame_start()
{
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
}

void GfxDevice::frame_end()
{
}

void GfxDevice::compile_and_create_shader(ShaderStage stage, const std::filesystem::path& fpath, Shader* shader)
{
	ShaderBytecode bc;
	compile_shader(stage, fpath, &bc);
	create_shader(stage, bc, shader);
}

void GfxDevice::compile_shader(ShaderStage stage, const std::filesystem::path& fpath, ShaderBytecode* bytecode)
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

	UINT flags = D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	BlobPtr shader_blob;
	BlobPtr error_blob;
	auto HR = D3DCompileFromFile(
		fpath.wstring().c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
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
			assert(false);
			return;
		}
	}
	
	bytecode->code = std::make_shared<std::vector<uint8_t>>();
	bytecode->code->resize(shader_blob->GetBufferSize());
	std::memcpy(bytecode->code->data(), shader_blob->GetBufferPointer(), shader_blob->GetBufferSize());
}

void GfxDevice::create_buffer(const BufferDesc& desc, GPUBuffer* buffer, std::optional<SubresourceData> subres)
{
	const auto& d3d_desc = desc.m_desc;

	buffer->m_type = desc.m_type;

	HRCHECK(m_dev->get_device()->CreateBuffer(
		&d3d_desc,
		subres ? &subres.value().m_subres : nullptr,
		(ID3D11Buffer**)buffer->m_internal_resource.ReleaseAndGetAddressOf()));

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

	texture->m_type = desc.m_type;

	// Grab misc. data
	bool is_array = d3d_desc.ArraySize > 1 ? true : false;
	bool ms_on = d3d_desc.SampleDesc.Count > 1 ? true : false;
	bool is_cube = d3d_desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE ? true : false;

	if (ms_on && d3d_desc.MipLevels != 1)
		assert(false);		// https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_texture2d_desc MipLevels = 1 required for MS

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
			subres ? &subres.value().m_subres : nullptr,
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
			m_dev->get_context()->GenerateMips(texture->m_srv.Get());

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
}


GPUTexture GfxDevice::get_backbuffer()
{
	return m_backbuffer;
}

void GfxDevice::create_framebuffer(const FramebufferDesc& desc, Framebuffer* framebuffer)
{
	framebuffer->m_depth_stencil_target = desc.m_depth_stencil_target.has_value() ? desc.m_depth_stencil_target.value() : GPUTexture();

	for (int i = 0; i < desc.m_targets.size(); ++i)
	{
		if (desc.m_targets[i].has_value())
			framebuffer->m_targets[i] = desc.m_targets[i].value();
	}

	framebuffer->m_is_registered = true;
}

void GfxDevice::create_pipeline(const PipelineDesc& desc, GraphicsPipeline* pipeline)
{
	auto& dev = m_dev->get_device();

	// create rasterizer state
	HRCHECK(dev->CreateRasterizerState1(&desc.m_rasterizer_desc.m_rasterizer_desc,
		(ID3D11RasterizerState1**)pipeline->m_rasterizer.m_internal_resource.ReleaseAndGetAddressOf()));

	// create blend state
	HRCHECK(dev->CreateBlendState1(&desc.m_blend_desc.m_blend_desc,
		(ID3D11BlendState1**)pipeline->m_blend.m_internal_resource.ReleaseAndGetAddressOf()));

	// create depth stencil state
	HRCHECK(dev->CreateDepthStencilState(&desc.m_depth_stencil_desc.m_depth_stencil_desc,
		(ID3D11DepthStencilState**)pipeline->m_depth_stencil.m_internal_resource.ReleaseAndGetAddressOf()));

	// create input layout (duplicates may be created here, we will ignore this for simplicity)
	HRCHECK(dev->CreateInputLayout(
		desc.m_input_desc.m_input_descs.data(),
		(UINT)desc.m_input_desc.m_input_descs.size(),
		desc.m_vs.m_blob.code->data(),
		(UINT)desc.m_vs.m_blob.code->size(),
		(ID3D11InputLayout**)pipeline->m_input_layout.m_internal_resource.ReleaseAndGetAddressOf()));

	// add shaders
	pipeline->m_vs = desc.m_vs;
	pipeline->m_ps = desc.m_ps;
	if (desc.m_gs.has_value())
		pipeline->m_gs = desc.m_gs.value();
	if (desc.m_hs.has_value())
		pipeline->m_hs = desc.m_hs.value();
	if (desc.m_ds.has_value())
		pipeline->m_ds = desc.m_ds.value();

	pipeline->m_is_registered = true;
}

void GfxDevice::draw()
{
	m_dev->get_context()->Draw(3, 0);
}

void GfxDevice::present(bool vsync)
{
	m_dev->get_sc()->Present(vsync ? 1 : 0, 0);
}

void GfxDevice::begin_pass(const Framebuffer* framebuffer, DepthStencilClear ds_clear)
{
	if (!framebuffer->m_is_registered)
	{
		assert(false && "Framebuffer is not registered!");
		return;
	}

	auto& ctx = m_dev->get_context();
	auto& dsv_opt = framebuffer->m_depth_stencil_target;

	// clear framebuffer and get render targets
	ID3D11RenderTargetView* rtvs[gfxconstants::MAX_RENDER_TARGETS] = {};
	for (int i = 0; i < framebuffer->m_targets.size(); ++i)
	{
		// get target
		auto& target = framebuffer->m_targets[i];
		if (!target.is_valid())
			break;
		rtvs[i] = target.m_rtv.Get();

		// clear target
		ctx->ClearRenderTargetView(target.m_rtv.Get(), target.m_desc.m_render_target_clear.m_rgba.data());
	}

	// clear depth stencil
	if (dsv_opt.is_valid())
	{
		auto dsv = dsv_opt.m_dsv.Get();
		ctx->ClearDepthStencilView(dsv, ds_clear.m_clear_flags, ds_clear.m_depth, ds_clear.m_stencil);	// clear ds
		ctx->OMSetRenderTargets(gfxconstants::MAX_RENDER_TARGETS, rtvs, dsv);	// bind targets (with dsv)
	}
	else
	{
		ctx->OMSetRenderTargets(gfxconstants::MAX_RENDER_TARGETS, rtvs, nullptr);	// bind targets (no dsv)
	}


}

void GfxDevice::end_pass()
{
	auto& ctx = m_dev->get_context();

	ctx->OMSetRenderTargets(gfxconstants::MAX_RENDER_TARGETS, (ID3D11RenderTargetView* const*)gfxconstants::NULL_RESOURCE, nullptr);

	// unbind UAVs too
	ctx->CSSetUnorderedAccessViews(0, gfxconstants::MAX_CS_UAV, (ID3D11UnorderedAccessView* const*)gfxconstants::NULL_RESOURCE, (const UINT*)gfxconstants::NULL_RESOURCE);

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

void GfxDevice::bind_pipeline(const GraphicsPipeline* pipeline, std::array<FLOAT, 4> blend_factor, UINT stencil_ref)
{
	if (!pipeline->m_is_registered)
	{
		assert(false);
		return;
	}

	auto& ctx = m_dev->get_context();

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

	// bind topo, raster state, depth-stencil, blend state
	ctx->IASetPrimitiveTopology(pipeline->m_topology);
	ctx->RSSetState((ID3D11RasterizerState*)pipeline->m_rasterizer.m_internal_resource.Get());
	ctx->OMSetDepthStencilState((ID3D11DepthStencilState*)pipeline->m_depth_stencil.m_internal_resource.Get(), stencil_ref);
	ctx->OMSetBlendState((ID3D11BlendState*)pipeline->m_blend.m_internal_resource.Get(), blend_factor.data(), pipeline->m_sample_mask);
}

