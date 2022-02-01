#pragma once
#include "Graphics/GfxTypes.h"
#include "Graphics/GfxDevice.h"

class TextureManager
{
public:
	static void initialize(GfxDevice* dev);
	static void shutdown();

	TextureManager(GfxDevice* dev);
	~TextureManager();

	GPUTexture* load_from_disk(const std::filesystem::path& fpath);
	void remove(const GPUTexture* texture);

private:
	GfxDevice* m_dev;

	std::map<void*, GPUTexture> m_textures;

	/*
		Removing a texture requires iterating over this map to find the corresponding internal resource..
		We'll find a better solution if we NEED it.
	*/
	std::map<std::filesystem::path, void*> m_path_mapper;

};

