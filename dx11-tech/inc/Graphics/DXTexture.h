#pragma once
#include "Graphics/DXResource.h"
#include <functional>

class DXTexture : public DXResource
{
public:
	DXTexture(DXDevice* dev, const TextureDesc& desc);
	DXTexture() = delete;
	~DXTexture() = default;
	DXTexture(const DXTexture& rh);

	void create_srv(DXDevice* dev, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc);
	void create_uav(DXDevice* dev, const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc);
	void create_rtv(DXDevice* dev, const D3D11_RENDER_TARGET_VIEW_DESC& desc);

	void create_srv_ext(DXDevice* dev, std::variant<
		std::function<D3D11_SHADER_RESOURCE_VIEW_DESC(ID3D11Texture1D*)>,
		std::function<D3D11_SHADER_RESOURCE_VIEW_DESC(ID3D11Texture2D*)>,
		std::function<D3D11_SHADER_RESOURCE_VIEW_DESC(ID3D11Texture3D*)>> extended_desc);

	void create_uav_ext(DXDevice* dev, std::variant<
		std::function<D3D11_UNORDERED_ACCESS_VIEW_DESC(ID3D11Texture1D*)>,
		std::function<D3D11_UNORDERED_ACCESS_VIEW_DESC(ID3D11Texture2D*)>,
		std::function<D3D11_UNORDERED_ACCESS_VIEW_DESC(ID3D11Texture3D*)>> extended_desc);

	void create_rtv_ext(DXDevice* dev, std::variant<
		std::function<D3D11_RENDER_TARGET_VIEW_DESC(ID3D11Texture1D*)>,
		std::function<D3D11_RENDER_TARGET_VIEW_DESC(ID3D11Texture2D*)>,
		std::function<D3D11_RENDER_TARGET_VIEW_DESC(ID3D11Texture3D*)>> extended_desc);

	operator ID3D11RenderTargetView* () { get_rtv(); }

	const TextureDesc& get_desc() { return m_desc; }

private:
	ID3D11RenderTargetView* get_rtv() const;

	template <typename Desc, typename Tex1D, typename Tex2D, typename Tex3D>
	void create_view_ext(DXDevice* dev, std::variant<
		std::function<Desc(Tex1D*)>,
		std::function<Desc(Tex2D*)>,
		std::function<Desc(Tex3D*)>> extended_desc, std::function<void(DXDevice*, const Desc&)> view_creator)
	{
		// 1D
		if (extended_desc.index() == 0)
		{
			auto fn = std::get<0>(extended_desc);
			view_creator(dev, fn(get_1d()));
		}
		// 2D
		else if (extended_desc.index() == 1)
		{
			auto fn = std::get<1>(extended_desc);
			view_creator(dev, fn(get_2d()));
		}
		// 3D
		else if (extended_desc.index() == 2)
		{
			auto fn = std::get<2>(extended_desc);
			view_creator(dev, fn(get_3d()));
		}
	}

	ID3D11Texture1D* get_1d();
	ID3D11Texture2D* get_2d();
	ID3D11Texture3D* get_3d();

	DXGI_FORMAT get_texture_format();



private:
	TextureDesc m_desc;
	RtvPtr m_rtv;
};

