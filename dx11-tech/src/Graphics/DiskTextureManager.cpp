#include "pch.h"
#include "Graphics/DiskTextureManager.h"

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

GPUTexture* DiskTextureManager::load_from(const std::filesystem::path& fpath)
{
	auto it = m_path_mapper.find(fpath);
	if (it != m_path_mapper.cend())
	{
		// Texture already exists
		return &m_textures.find(it->second)->second;
	}

	// Load with STB Image 
	int width = 0;
	int height = 0;
	int channels = 0;
	auto image_data = stbi_load(fpath.string().c_str(), &width, &height, &channels, 4);
	int row_in_bytes = width * 4;		// width * 4 bytes (R8G8B8A8)

	// If failed to load..
	if (width == 0 || height == 0)
	{
		return nullptr;
	}

	// Always assuming SRGB
	auto desc = TextureDesc::make_2d(
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, width, height, 
		D3D11_BIND_SHADER_RESOURCE, 0, 1, D3D11_USAGE_DEFAULT, 0, 1, 0,
		D3D11_RESOURCE_MISC_GENERATE_MIPS);
	
	// Create texture
	GPUTexture new_texture;
	m_dev->create_texture(desc, &new_texture, SubresourceData(image_data, row_in_bytes, 0));

	// Free data from host
	stbi_image_free(image_data);

	// Track using the internal resource
	void* internal_resource = new_texture.m_internal_resource.Get();
	m_path_mapper.insert({ fpath, internal_resource });

	// Insert to persistent textures map and get texture
	auto tex_it = m_textures.insert({ internal_resource, new_texture });
	GPUTexture* tex = &tex_it.first->second;

	return tex;
}

void DiskTextureManager::remove(const GPUTexture* texture)
{
	auto internal_res = texture->m_internal_resource.Get();

	// Remove from textures
	auto tex_it = m_textures.find(internal_res);
	if (tex_it == m_textures.cend())	
		return;		// exit early if no textures found
	m_textures.erase(internal_res);

	// Remove from path mapper
	std::map<std::filesystem::path, void*>::iterator del_it;
	for (auto it = m_path_mapper.begin(); it != m_path_mapper.end(); ++it)
	{
		if (it->second == internal_res)
		{
			del_it = it;
			break;
		}
	}
	m_path_mapper.erase(del_it);
	
	/*
		For the resource to be deleted, all the outstanding external references must be gone.
		It is not at least not persistently stored in the global texture list.
		Responsibility is up to the user after this to ensure that the texture is actually gone.
	*/
}
