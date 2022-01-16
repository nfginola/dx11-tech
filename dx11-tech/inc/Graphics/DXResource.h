#pragma once
#include "GfxCommon.h"

class DXResource
{
public:
	DXResource() = default;
	~DXResource() = default;
	DXResource& operator=(const DXResource&) = delete;
	DXResource(const DXResource& rh);

	operator ID3D11ShaderResourceView* () { return get_srv(); }
	operator ID3D11UnorderedAccessView* () { return get_uav(); }

	// Copy from CPU to a mappable GPU memory
	void update_map(DXDevice* dev, void* data, UINT size, UINT subres = 0, D3D11_MAP map_type = D3D11_MAP_WRITE_DISCARD, UINT map_flags = 0);

	// Copy from CPU to a non-mappable GPU memory
	void update_subres(DXDevice* dev, void* data, UINT src_row_pitch, UINT src_depth_pitch, UINT dest_subres, const D3D11_BOX& dest_box);

private:
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

