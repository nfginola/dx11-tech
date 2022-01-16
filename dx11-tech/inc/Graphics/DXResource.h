#pragma once
#include "GfxCommon.h"

class DXResource
{
public:
	DXResource() = default;
	~DXResource() = default;
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

