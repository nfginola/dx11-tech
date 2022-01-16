#include "pch.h"
#include "Graphics/DXResource.h"


void DXResource::update_map(DXDevice* dev, void* data, UINT size, UINT subres, D3D11_MAP map_type, UINT map_flags)
{
	D3D11_MAPPED_SUBRESOURCE mapped{};
	dev->get_context()->Map(m_res.Get(), subres, map_type, map_flags, &mapped);
	std::memcpy(mapped.pData, data, size);
	dev->get_context()->Unmap(m_res.Get(), subres);
}

void DXResource::update_subres(DXDevice* dev, void* data, UINT src_row_pitch, UINT src_depth_pitch, UINT dest_subres, const D3D11_BOX& dest_box)
{
	dev->get_context()->UpdateSubresource1(m_res.Get(), dest_subres, &dest_box, data, src_row_pitch, src_depth_pitch, D3D11_COPY_DISCARD);
}

ID3D11ShaderResourceView* DXResource::get_srv() const
{
	if (!m_srv)
		assert(false);
	return m_srv.Get();
}

ID3D11UnorderedAccessView* DXResource::get_uav() const
{
	if (!m_uav)
		assert(false);
	return m_uav.Get();
}

DXResource::DXResource(const DXResource& rh)
{
	// When a resource is "copied", it simply increase the reference.	
	m_res = rh.m_res;
}
