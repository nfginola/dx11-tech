#include "pch.h"
#include "Graphics/GfxApi.h"
#include "Graphics/GfxCommon.h"

GfxApi::GfxApi(std::unique_ptr<DXDevice> dev) :
	m_dev(std::move(dev))
{

}

GfxApi::~GfxApi()
{

}

void GfxApi::create_buffer(const BufferDesc& desc, GPUBuffer* buffer, std::optional<SubresourceData> subres)
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

void GfxApi::create_texture(const TextureDesc& desc, GPUTexture* texture, std::optional<SubresourceData> subres)
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

void GfxApi::create_framebuffer(const FramebufferDesc& desc, Framebuffer* framebuffer)
{
	framebuffer->m_depth_stencil = desc.m_depth_stencil;
	framebuffer->m_targets = desc.m_targets;
	framebuffer->m_is_registered = true;
}

void GfxApi::begin_pass(const Framebuffer* framebuffer,
	const std::vector<D3D11_VIEWPORT>& viewports,
	std::optional<DepthStencilClear> m_ds_clear)
{
	if (!framebuffer->m_is_registered)
	{
		assert(false && "Framebuffer is not registered!");
		return;
	}

	auto& ctx = m_dev->get_context();
	auto& dsv_opt = framebuffer->m_depth_stencil;

	ID3D11RenderTargetView* rtvs[GfxConstants::MAX_RENDER_TARGETS] = {};

	// clear framebuffer (automatic clear to black if none supplied)
	// and get render targets
	for (int i = 0; i < framebuffer->m_targets.size(); ++i)
	{
		// get target
		auto& target_opt = framebuffer->m_targets[i];
		if (!target_opt.has_value())
			break;
		auto& target = target_opt.value();
		rtvs[i] = target.m_rtv.Get();					

		// clear target
		ctx->ClearRenderTargetView(target.m_rtv.Get(), target.m_desc.m_render_target_clear.m_rgba.data());
	}

	// clear depth stencil
	if (m_ds_clear.has_value() && dsv_opt.has_value())
	{
		auto& ds_clear = m_ds_clear.value();
		auto dsv = dsv_opt.value().m_dsv.Get();
		ctx->ClearDepthStencilView(dsv, ds_clear.m_clear_flags, ds_clear.m_depth, ds_clear.m_stencil);	// clear ds
		ctx->OMSetRenderTargets(GfxConstants::MAX_RENDER_TARGETS, rtvs, dsv);	// bind targets (with dsv)
	}
	else
	{
		ctx->OMSetRenderTargets(GfxConstants::MAX_RENDER_TARGETS, rtvs, nullptr);	// bind targets (no dsv)
	}

	// bind viewports
	ctx->RSSetViewports((UINT)viewports.size(), viewports.data());
}
