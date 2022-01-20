#include "pch.h"
#include "Graphics/GfxDevice.h"
#include "Graphics/GfxCommon.h"

static GfxDevice* s_device = nullptr;

namespace GfxConstants
{
	// For unbinding state
	const void* const NULL_RESOURCE[GfxConstants::MAX_SHADER_INPUT_RESOURCE_SLOTS] = {};
}

void GfxDevice::initialize(unique_ptr<DXDevice> dev)
{
	if (!s_device)
		s_device = new GfxDevice(std::move(dev));
	else
		assert(false);	// dont try initializing multiple times..
}

void GfxDevice::shutdown()
{
	if (s_device)
		delete s_device;
}

GfxDevice* GfxDevice::get()
{
	return s_device;
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

void GfxDevice::begin_frame()
{
	auto& ctx = m_dev->get_context();
	ctx->RSSetScissorRects(GfxConstants::MAX_SCISSORS, (const D3D11_RECT*)GfxConstants::NULL_RESOURCE);

	// nuke all SRVs
	// https://stackoverflow.com/questions/20300778/are-there-directx-guidelines-for-binding-and-unbinding-resources-between-draw-ca
	ctx->VSSetShaderResources(0, GfxConstants::MAX_SHADER_INPUT_RESOURCE_SLOTS - 64, (ID3D11ShaderResourceView* const*)GfxConstants::NULL_RESOURCE);
	ctx->HSSetShaderResources(0, GfxConstants::MAX_SHADER_INPUT_RESOURCE_SLOTS - 64, (ID3D11ShaderResourceView* const*)GfxConstants::NULL_RESOURCE);
	ctx->DSSetShaderResources(0, GfxConstants::MAX_SHADER_INPUT_RESOURCE_SLOTS - 64, (ID3D11ShaderResourceView* const*)GfxConstants::NULL_RESOURCE);
	ctx->GSSetShaderResources(0, GfxConstants::MAX_SHADER_INPUT_RESOURCE_SLOTS - 64, (ID3D11ShaderResourceView* const*)GfxConstants::NULL_RESOURCE);
	ctx->PSSetShaderResources(0, GfxConstants::MAX_SHADER_INPUT_RESOURCE_SLOTS - 64, (ID3D11ShaderResourceView* const*)GfxConstants::NULL_RESOURCE);
	ctx->CSSetShaderResources(0, GfxConstants::MAX_SHADER_INPUT_RESOURCE_SLOTS - 64, (ID3D11ShaderResourceView* const*)GfxConstants::NULL_RESOURCE);
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
	// Create...

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

	// unbind all SRVs (adds overhead (how much?), but adds safety)


	ID3D11RenderTargetView* rtvs[GfxConstants::MAX_RENDER_TARGETS] = {};

	// clear framebuffer (automatic clear to black if none supplied)
	// and get render targets
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
		ctx->OMSetRenderTargets(GfxConstants::MAX_RENDER_TARGETS, rtvs, dsv);	// bind targets (with dsv)
	}
	else
	{
		ctx->OMSetRenderTargets(GfxConstants::MAX_RENDER_TARGETS, rtvs, nullptr);	// bind targets (no dsv)
	}


}

void GfxDevice::end_pass()
{
	auto& ctx = m_dev->get_context();

	ctx->OMSetRenderTargets(GfxConstants::MAX_RENDER_TARGETS, (ID3D11RenderTargetView* const*)GfxConstants::NULL_RESOURCE, nullptr);

	// unbind UAVs too
	ctx->CSSetUnorderedAccessViews(0, GfxConstants::MAX_CS_UAV, (ID3D11UnorderedAccessView* const*)GfxConstants::NULL_RESOURCE, (const UINT*)GfxConstants::NULL_RESOURCE);

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

