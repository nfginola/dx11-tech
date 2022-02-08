#include "pch.h"
#include "Graphics/MaterialManager.h"
#include "Graphics/DiskTextureManager.h"

namespace gfx { MaterialManager* mat_mgr = nullptr; }

void MaterialManager::initialize(DiskTextureManager* disk_tex_mgr)
{
	if (!gfx::mat_mgr)
		gfx::mat_mgr = new MaterialManager(disk_tex_mgr);
}

void MaterialManager::shutdown()
{
	if (gfx::mat_mgr)
	{
		delete gfx::mat_mgr;
		gfx::mat_mgr = nullptr;
	}
}

MaterialManager::MaterialManager(DiskTextureManager* disk_tex_mgr) :
	m_disk_tex_mgr(disk_tex_mgr)
{
}

const Material* MaterialManager::load_material(const AssimpMaterialData& mat_data, const std::string& name)
{
	if (m_mats.find(name) != m_mats.cend())
		assert(false);		// Name already taken

	Material* mat_ret = nullptr;

	// Handle material
	const auto& paths = std::get<AssimpMaterialData::PhongPaths>(mat_data.file_paths);

	// Load textures
	auto diffuse = m_disk_tex_mgr->load_from(paths.diffuse);

	// Create material
	auto mat = Material().
		set_texture(Material::Texture::eAlbedo, diffuse);

	// Check if material exists
	auto it = std::find_if(m_mats.begin(), m_mats.end(), [&](const auto& pair) { return mat == pair.second; });
	if (it == m_mats.end())
	{
		std::string mat_name = name;
		if (mat_name.empty())
			mat_name = "Mat" + std::to_string(m_def_counter++);

		// Save material
		auto it = m_mats.insert({ mat_name, mat });
		mat_ret = &(it.first->second);
	}
	else
	{
		// Get existing material
		mat_ret = &(it->second);
	}

	return mat_ret;
}

const Material* MaterialManager::get_material(const std::string& name)
{
	return &(m_mats.find(name)->second);
}

void MaterialManager::remove_material(const std::string& name)
{
	m_mats.erase(name);
}
