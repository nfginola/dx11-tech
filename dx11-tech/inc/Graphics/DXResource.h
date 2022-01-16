#pragma once
#include "GfxCommon.h"

class DXResource
{
public:
	DXResource() = default;
	~DXResource() = default;

	//void create_uav(DXDevice* dev, const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc);
	//void create_srv(DXDevice* dev, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc);





	//template <typename T>
	//T* get_resource() const
	//{
	//	return static_cast<T*>(m_res.Get());
	//};
	ID3D11ShaderResourceView* get_srv() const;
	ID3D11UnorderedAccessView* get_uav() const;

protected:
	GPUResourcePtr m_res;
	SrvPtr m_srv;
	UavPtr m_uav;

	// if write is bound --> check tuple to unbind
	// if write is not bound --> no need to do anything
	//std::pair<bool, std::tuple<uint8_t, GPUAccess, ShaderStage>> m_is_write_bound = { false, {} };

};

