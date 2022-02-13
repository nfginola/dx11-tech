#include "pch.h"
#include "Graphics/DiskTextureManager.h"
#include "Graphics/API/GfxDevice.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace gfx { DiskTextureManager* tex_mgr = nullptr; }

void DiskTextureManager::initialize(GfxDevice* dev)
{
	if (!gfx::tex_mgr)
		gfx::tex_mgr = new DiskTextureManager(dev);
}

void DiskTextureManager::shutdown()
{
	if (gfx::tex_mgr)
	{
		delete gfx::tex_mgr;
		gfx::tex_mgr = nullptr;
	}
}

DiskTextureManager::DiskTextureManager(GfxDevice* dev) :
	m_dev(dev)
{
}

DiskTextureManager::~DiskTextureManager()
{
}

TextureHandle DiskTextureManager::load_from(const std::filesystem::path& fpath)
{
	auto it = m_path_to_tex.find(fpath);
	if (it != m_path_to_tex.end())
		return it->second;

	// Load with STB Image 
	int width = 0;
	int height = 0;
	int channels = 0;
	auto image_data = stbi_load(fpath.string().c_str(), &width, &height, &channels, 4);
	int row_in_bytes = width * 4;		// width * 4 bytes (R8G8B8A8)

	// If failed to load..
	if (width == 0 || height == 0)
	{
		return TextureHandle{0};
	}

	// Always assuming SRGB
	auto desc = TextureDesc::make_2d(
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, width, height, 
		D3D11_BIND_SHADER_RESOURCE, 0, 1, D3D11_USAGE_DEFAULT, 0, 1, 0,
		D3D11_RESOURCE_MISC_GENERATE_MIPS);
	
	auto tex = m_dev->create_texture(desc, SubresourceData(image_data, row_in_bytes, 0));

	// Free data from host
	stbi_image_free(image_data);
	
	m_path_to_tex.insert({ fpath, tex });
	m_tex_to_path.insert({ tex, fpath });

	return tex;
}

void DiskTextureManager::remove(TextureHandle texture)
{
	// Delete from bimap

	auto it = m_tex_to_path.find(texture);

	if (it == m_tex_to_path.end())
		return; // Didn't find the texture

	const auto associated_path = it->second;
	m_tex_to_path.erase(texture);
	m_path_to_tex.erase(associated_path);
}
