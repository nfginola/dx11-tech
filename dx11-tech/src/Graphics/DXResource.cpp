#include "pch.h"
#include "Graphics/DXResource.h"

//
//void DXResource::create_uav(DXDevice* dev, const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc)
//{
//	HRCHECK(dev->get_device()->CreateUnorderedAccessView(m_res.Get(), &desc, m_uav.GetAddressOf()));
//}
//
//void DXResource::create_srv(DXDevice* dev, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
//{
//	HRCHECK(dev->get_device()->CreateShaderResourceView(m_res.Get(), &desc, m_srv.GetAddressOf()));
//}

ID3D11ShaderResourceView* DXResource::get_srv() const
{
	return m_srv.Get();
}

ID3D11UnorderedAccessView* DXResource::get_uav() const
{
	return m_uav.Get();
}
