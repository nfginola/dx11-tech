#pragma once
#include "Graphics/DXResource.h"

class DXTexture : public DXResource
{
public:
	DXTexture(DXDevice* dev, const D3D11_TEXTURE1D_DESC& desc, const D3D11_SUBRESOURCE_DATA& subres);
	DXTexture(DXDevice* dev, const D3D11_TEXTURE2D_DESC& desc, const D3D11_SUBRESOURCE_DATA& subres);
	DXTexture(DXDevice* dev, const D3D11_TEXTURE3D_DESC& desc, const D3D11_SUBRESOURCE_DATA& subres);
	~DXTexture() = default;

	void create_rtv(DXDevice* dev, const D3D11_RENDER_TARGET_VIEW_DESC& desc);
	const RtvPtr& get_rtv() const;

private:
	RtvPtr m_rtv;
};

