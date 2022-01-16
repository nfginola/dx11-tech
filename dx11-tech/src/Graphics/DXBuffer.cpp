#include "pch.h"
#include "Graphics/DXBuffer.h"

DXBuffer::DXBuffer(DXDevice* dev, const BufferDesc& desc) :
	m_desc(desc)
{
	if (desc.subres.pSysMem != nullptr)
		HRCHECK(dev->get_device()->CreateBuffer(&desc.desc, &desc.subres, (ID3D11Buffer**)m_res.GetAddressOf()));
	else 
		HRCHECK(dev->get_device()->CreateBuffer(&desc.desc, nullptr, (ID3D11Buffer**)m_res.GetAddressOf()));
}

DXBuffer::DXBuffer(const DXBuffer& rh) :
	DXResource::DXResource(rh)
{
	m_desc = rh.m_desc;
}

void DXBuffer::create_srv(DXDevice* dev, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
{
	if (!view_allowed())
		return;

	switch (m_desc.type)
	{
	case BufferType::eRaw:
		if (desc.ViewDimension != D3D11_SRV_DIMENSION_BUFFEREX)
		{
			assert(false);
		}
		break;
	case BufferType::eAppendConsume:
	case BufferType::eByteAddress:
	case BufferType::eStructured:
		if (desc.ViewDimension != D3D11_SRV_DIMENSION_BUFFER)
		{
			assert(false);
		}
		break;
	case BufferType::eNone:
		assert(false);
		break;
	}

	HRCHECK(dev->get_device()->CreateShaderResourceView(m_res.Get(), &desc, m_srv.GetAddressOf()));
}

void DXBuffer::create_uav(DXDevice* dev, const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc)
{
	if (!view_allowed())
		return;

	switch (m_desc.type)
	{
	case BufferType::eRaw:
	case BufferType::eAppendConsume:
	case BufferType::eByteAddress:
	case BufferType::eStructured:
		if (desc.ViewDimension != D3D11_UAV_DIMENSION_BUFFER)
		{
			assert(false);
		}
		break;
	case BufferType::eNone:
		assert(false);
		break;
	}

	HRCHECK(dev->get_device()->CreateUnorderedAccessView(m_res.Get(), &desc, m_uav.GetAddressOf()));
}

bool DXBuffer::view_allowed()
{
	if (m_desc.type == BufferType::eConstant ||
		m_desc.type == BufferType::eVertex ||
		m_desc.type == BufferType::eIndex)
	{
		return false;
	}
	return true;
}
