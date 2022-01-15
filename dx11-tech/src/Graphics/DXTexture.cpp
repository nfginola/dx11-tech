#include "pch.h"
#include "Graphics/DXTexture.h"


DXTexture::DXTexture(DXDevice* dev, const D3D11_TEXTURE1D_DESC& desc, const D3D11_SUBRESOURCE_DATA& subres)
{
	HRCHECK(dev->get_device()->CreateTexture1D(&desc, &subres, (ID3D11Texture1D**)m_res.GetAddressOf()));
}

DXTexture::DXTexture(DXDevice* dev, const D3D11_TEXTURE2D_DESC& desc, const D3D11_SUBRESOURCE_DATA& subres)
{
	HRCHECK(dev->get_device()->CreateTexture2D(&desc, &subres, (ID3D11Texture2D**)m_res.GetAddressOf()));
}

DXTexture::DXTexture(DXDevice* dev, const D3D11_TEXTURE3D_DESC& desc, const D3D11_SUBRESOURCE_DATA& subres)
{
	HRCHECK(dev->get_device()->CreateTexture3D(&desc, &subres, (ID3D11Texture3D**)m_res.GetAddressOf()));
}

void DXTexture::create_rtv(DXDevice* dev, const D3D11_RENDER_TARGET_VIEW_DESC& desc)
{
	HRCHECK(dev->get_device()->CreateRenderTargetView(m_res.Get(), &desc, m_rtv.GetAddressOf()));
}

const RtvPtr& DXTexture::get_rtv() const
{
	return m_rtv;
}
