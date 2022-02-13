#pragma once
//#include "Graphics/API/GfxTypes.h"
#include "Graphics/API/GfxDevice.h"

class DiskTextureManager
{
public:
	static void initialize(GfxDevice* dev);
	static void shutdown();

	DiskTextureManager(GfxDevice* dev);
	~DiskTextureManager();

	TextureHandle load_from(const std::filesystem::path& fpath);
	void remove(TextureHandle tex);

private:
	GfxDevice* m_dev;

	// Bidirectional hash map
	std::map<TextureHandle, std::filesystem::path> m_tex_to_path;
	std::map<std::filesystem::path, TextureHandle> m_path_to_tex;



};

