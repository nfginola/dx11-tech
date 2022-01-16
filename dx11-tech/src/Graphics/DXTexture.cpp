#include "pch.h"
#include "Graphics/DXTexture.h"

DXTexture::DXTexture(DXDevice* dev, const TextureDesc& desc) :
	m_desc(desc)
{
	auto& d = desc.desc;
	auto& subres = desc.subres;

	if (std::holds_alternative<D3D11_TEXTURE1D_DESC>(d))
	{
		const D3D11_TEXTURE1D_DESC& tex_desc = std::get<D3D11_TEXTURE1D_DESC>(d);

		if (subres.pSysMem != nullptr)
			HRCHECK(dev->get_device()->CreateTexture1D(&tex_desc, &desc.subres, (ID3D11Texture1D**)m_res.GetAddressOf()));
		else
			HRCHECK(dev->get_device()->CreateTexture1D(&tex_desc, nullptr, (ID3D11Texture1D**)m_res.GetAddressOf()));
	}
	else if (std::holds_alternative<D3D11_TEXTURE2D_DESC>(d))
	{
		const D3D11_TEXTURE2D_DESC& tex_desc = std::get<D3D11_TEXTURE2D_DESC>(d);

		if (subres.pSysMem != nullptr)
			HRCHECK(dev->get_device()->CreateTexture2D(&tex_desc, &desc.subres, (ID3D11Texture2D**)m_res.GetAddressOf()));
		else
			HRCHECK(dev->get_device()->CreateTexture2D(&tex_desc, nullptr, (ID3D11Texture2D**)m_res.GetAddressOf()));
	}
	else if (std::holds_alternative<D3D11_TEXTURE3D_DESC>(d))
	{
		const D3D11_TEXTURE3D_DESC& tex_desc = std::get<D3D11_TEXTURE3D_DESC>(d);
		if (subres.pSysMem != nullptr)
			HRCHECK(dev->get_device()->CreateTexture3D(&tex_desc, &desc.subres, (ID3D11Texture3D**)m_res.GetAddressOf()));
		else
			HRCHECK(dev->get_device()->CreateTexture3D(&tex_desc, nullptr, (ID3D11Texture3D**)m_res.GetAddressOf()));
	}
}

void DXTexture::create_rtv(DXDevice* dev, const D3D11_RENDER_TARGET_VIEW_DESC& desc)
{
	switch (m_desc.type)
	{
	case TextureType::e1D:
		if (desc.ViewDimension != D3D11_RTV_DIMENSION_TEXTURE1D &&
			desc.ViewDimension != D3D11_RTV_DIMENSION_TEXTURE1DARRAY)
		{
			assert(false);
		}
		break;
	case TextureType::e2D:
		if (desc.ViewDimension != D3D11_RTV_DIMENSION_TEXTURE2D &&
			desc.ViewDimension != D3D11_RTV_DIMENSION_TEXTURE2DARRAY &&
			desc.ViewDimension != D3D11_RTV_DIMENSION_TEXTURE2DMS &&
			desc.ViewDimension != D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY)
		{
			assert(false);
		}
		break;
	case TextureType::e3D:
		if (desc.ViewDimension != D3D11_RTV_DIMENSION_TEXTURE3D)
		{
			assert(false);
		}
		break;
	}

	HRCHECK(dev->get_device()->CreateRenderTargetView(m_res.Get(), &desc, m_rtv.GetAddressOf()));
}

void DXTexture::create_uav(DXDevice* dev, const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc)
{
	switch (m_desc.type)
	{
	case TextureType::e1D:
		if (desc.ViewDimension != D3D11_UAV_DIMENSION_TEXTURE1D &&
			desc.ViewDimension != D3D11_UAV_DIMENSION_TEXTURE1DARRAY)
		{
			assert(false);
		}
		break;
	case TextureType::e2D:
		if (desc.ViewDimension != D3D11_UAV_DIMENSION_TEXTURE2D &&
			desc.ViewDimension != D3D11_UAV_DIMENSION_TEXTURE2DARRAY)
		{
			assert(false);
		}
		break;
	case TextureType::e3D:
		if (desc.ViewDimension != D3D11_UAV_DIMENSION_TEXTURE3D)
		{
			assert(false);
		}
		break;
	}

	HRCHECK(dev->get_device()->CreateUnorderedAccessView(m_res.Get(), &desc, m_uav.GetAddressOf()));
}

void DXTexture::create_srv(DXDevice* dev, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
{
	switch (m_desc.type)
	{
	case TextureType::e1D:
		if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE1D &&
			desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE1DARRAY)
		{
			assert(false);
		}
		break;
	case TextureType::e2D:
		if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D &&
			desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2DARRAY &&
			desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2DMS &&
			desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY &&
			desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURECUBE && 
			desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURECUBEARRAY)
		{
			assert(false);
		}
		break;
	case TextureType::e3D:
		if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE3D)
		{
			assert(false);
		}
		break;
	}

	HRCHECK(dev->get_device()->CreateShaderResourceView(m_res.Get(), &desc, m_srv.GetAddressOf()));
}

void DXTexture::create_srv_ext(DXDevice* dev, std::variant<
	std::function<D3D11_SHADER_RESOURCE_VIEW_DESC(ID3D11Texture1D*)>,
	std::function<D3D11_SHADER_RESOURCE_VIEW_DESC(ID3D11Texture2D*)>,
	std::function<D3D11_SHADER_RESOURCE_VIEW_DESC(ID3D11Texture3D*)>> extended_desc)
{
	std::function<void(DXDevice*, const D3D11_SHADER_RESOURCE_VIEW_DESC&)> creator = [&](DXDevice* dev, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc) { create_srv(dev, desc); };
	create_view_ext(dev, extended_desc, creator);
}

void DXTexture::create_uav_ext(DXDevice* dev, std::variant<
	std::function<D3D11_UNORDERED_ACCESS_VIEW_DESC(ID3D11Texture1D*)>, 
	std::function<D3D11_UNORDERED_ACCESS_VIEW_DESC(ID3D11Texture2D*)>, 
	std::function<D3D11_UNORDERED_ACCESS_VIEW_DESC(ID3D11Texture3D*)>> extended_desc)
{
	std::function<void(DXDevice*, const D3D11_UNORDERED_ACCESS_VIEW_DESC&)> creator = [&](DXDevice* dev, const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc) { create_uav(dev, desc); };
	create_view_ext(dev, extended_desc, creator);
}

void DXTexture::create_rtv_ext(DXDevice* dev, std::variant<
	std::function<D3D11_RENDER_TARGET_VIEW_DESC(ID3D11Texture1D*)>, 
	std::function<D3D11_RENDER_TARGET_VIEW_DESC(ID3D11Texture2D*)>, 
	std::function<D3D11_RENDER_TARGET_VIEW_DESC(ID3D11Texture3D*)>> extended_desc)
{
	std::function<void(DXDevice*, const D3D11_RENDER_TARGET_VIEW_DESC&)> creator = [&](DXDevice* dev, const D3D11_RENDER_TARGET_VIEW_DESC& desc) { create_rtv(dev, desc); };
	create_view_ext(dev, extended_desc, creator);
}

const RtvPtr& DXTexture::get_rtv() const
{
	return m_rtv;
}

ID3D11Texture1D* DXTexture::get_1d()
{
	if (m_desc.type == TextureType::e1D) 
		return reinterpret_cast<ID3D11Texture1D*>(m_res.Get());
	else
	{
		assert(false);
		return nullptr;
	}
}

ID3D11Texture2D* DXTexture::get_2d()
{
	if (m_desc.type == TextureType::e2D)
		return reinterpret_cast<ID3D11Texture2D*>(m_res.Get());
	else
	{
		assert(false);
		return nullptr;
	}
}

ID3D11Texture3D* DXTexture::get_3d()
{
	if (m_desc.type == TextureType::e3D)
		return reinterpret_cast<ID3D11Texture3D*>(m_res.Get());
	else
	{
		assert(false);
		return nullptr;
	}
}
