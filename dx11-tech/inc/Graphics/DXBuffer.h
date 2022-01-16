#pragma once
#include "Graphics/DXResource.h"

class DXBuffer : public DXResource
{
public:
	DXBuffer(DXDevice* dev, const BufferDesc& desc);
	DXBuffer() = delete;
	~DXBuffer() = default;
	DXBuffer(const DXBuffer& rh);

	void create_srv(DXDevice* dev, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc);
	void create_uav(DXDevice* dev, const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc);

	bool view_allowed();

	const BufferDesc& get_desc() { return m_desc; }

	operator ID3D11Buffer* () const { return get(); }

private:
	ID3D11Buffer* get() const { return reinterpret_cast<ID3D11Buffer*>(m_res.Get()); }

private:
	BufferDesc m_desc;

};

