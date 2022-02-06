#pragma once
#include "Graphics/GfxTypes.h"
#include "Graphics/GfxDevice.h"

class DiskTextureManager
{
public:
	static void initialize(GfxDevice* dev);
	static void shutdown();

	DiskTextureManager(GfxDevice* dev);
	~DiskTextureManager();

	GPUTexture* load_from(const std::filesystem::path& fpath);
	void remove(const GPUTexture* texture);

private:
	GfxDevice* m_dev;

	std::map<void*, GPUTexture> m_textures;

	/*
		Removing a texture requires iterating over this map to find the corresponding internal resource.. worst case O(n)
		We'll find a better solution if we NEED it.
	*/
	std::map<std::filesystem::path, void*> m_path_mapper;

};

