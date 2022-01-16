#pragma once
#include "Graphics/DXResource.h"

class DXTexture : public DXResource
{
public:
	DXTexture(DXDevice* dev, const TextureDesc& desc);
	DXTexture() = delete;
	~DXTexture() = default;

	void create_uav(DXDevice* dev, const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc);
	void create_srv(DXDevice* dev, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc);
	void create_rtv(DXDevice* dev, const D3D11_RENDER_TARGET_VIEW_DESC& desc);

	void create_srv_ext(DXDevice* dev, std::variant<
		std::function<D3D11_SHADER_RESOURCE_VIEW_DESC(ID3D11Texture1D*)>,
		std::function<D3D11_SHADER_RESOURCE_VIEW_DESC(ID3D11Texture2D*)>,
		std::function<D3D11_SHADER_RESOURCE_VIEW_DESC(ID3D11Texture3D*)>> extended_desc);

	void create_srv_ext(DXDevice* dev, std::function<D3D11_SHADER_RESOURCE_VIEW_DESC(ID3D11Texture1D*)> extended_desc);
	void create_srv_ext(DXDevice* dev, std::function<D3D11_SHADER_RESOURCE_VIEW_DESC(ID3D11Texture2D*)> extended_desc);
	void create_srv_ext(DXDevice* dev, std::function<D3D11_SHADER_RESOURCE_VIEW_DESC(ID3D11Texture3D*)> extended_desc);
		

	const RtvPtr& get_rtv() const;



private:
	operator ID3D11Texture1D* () { return get_1d(); };
	operator ID3D11Texture2D* () { return get_2d(); };
	operator ID3D11Texture3D* () { return get_3d(); };

	ID3D11Texture1D* get_1d();
	ID3D11Texture2D* get_2d();
	ID3D11Texture3D* get_3d();

private:
	TextureDesc m_desc;
	RtvPtr m_rtv;
};

