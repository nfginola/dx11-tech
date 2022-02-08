#pragma once
#include <map>
#include "Graphics/Material.h"
#include "AssimpTypes.h"

class MaterialManager
{
public:
	static void initialize(class DiskTextureManager* disk_tex_mgr);
	static void shutdown();

	MaterialManager() = delete;

	const Material* load_material(const AssimpMaterialData& mat_data, const std::string& name = "");
	const Material* get_material(const std::string& name);
	void remove_material(const std::string& name);

private:
	MaterialManager(DiskTextureManager* disk_tex_mgr);

private:
	DiskTextureManager* m_disk_tex_mgr = nullptr;
	
	uint64_t m_def_counter = 0;
	std::map<std::string, Material> m_mats;
};

