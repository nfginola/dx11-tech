#pragma once
#include "GfxCommon.h"

class DXResource
{
public:
	DXResource() = default;
	~DXResource() = default;

	void create_uav(DXDevice* dev, const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc);
	void create_srv(DXDevice* dev, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc);

	const SrvPtr& get_srv() const;
	const UavPtr& get_uav() const;

protected:
	GPUResourcePtr m_res;
	SrvPtr m_srv;
	UavPtr m_uav;

	bool m_is_write_bound = false;

};

