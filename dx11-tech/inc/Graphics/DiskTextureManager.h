#pragma once
#include "Graphics/API/GfxHandles.h"

class DiskTextureManager
{
public:
	static void initialize(class GfxDevice* dev);
	static void shutdown();

	DiskTextureManager(GfxDevice* dev);
	~DiskTextureManager();

	TextureHandle load_from(const std::filesystem::path& fpath);
	void remove(TextureHandle tex);

private:
	GfxDevice* m_dev;

	// Bidirectional hash map
	std::unordered_map<TextureHandle, std::string> m_tex_to_path;
	std::unordered_map<std::string, TextureHandle> m_path_to_tex;



};

