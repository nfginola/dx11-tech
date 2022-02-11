#include "pch.h"
#include "AssimpLoader.h"
#include "Graphics/GfxDevice.h"
#include "Graphics/MaterialManager.h"
#include "Graphics/ModelManager.h"

namespace gfx { ModelManager* model_mgr = nullptr; }


void ModelManager::initialize(GfxDevice* dev, MaterialManager* disk_tex_mgr)
{
	if (!gfx::model_mgr)
		gfx::model_mgr = new ModelManager(dev, disk_tex_mgr);
}

void ModelManager::shutdown()
{
	if (gfx::model_mgr)
	{
		delete gfx::model_mgr;
		gfx::model_mgr = nullptr;
	}
}

ModelManager::ModelManager(GfxDevice* dev, MaterialManager* mat_mgr) :
	m_dev(dev),
	m_mat_mgr(mat_mgr)
{

}

const Model* ModelManager::load_model(const std::filesystem::path& path, const std::string& name)
{
	// If path already exists, return the model
	auto it = m_path_mapper.find(path);
	if (it != m_path_mapper.cend())
		return &(m_models.find(it->second)->second);

	if (m_models.find(name) != m_models.cend())
		assert(false);		// name already taken

	AssimpLoader loader(path);

	const auto& positions = loader.get_positions();
	const auto& uvs = loader.get_uvs();
	const auto& normals = loader.get_normals();
	const auto& indices = loader.get_indices();
	const auto& meshes = loader.get_meshes();
	const auto& mats = loader.get_materials();
	assert(meshes.size() == mats.size());

	GPUBuffer pos, uv, nor, idx;
	m_dev->create_buffer(BufferDesc::vertex(positions.size() * sizeof(positions[0])), &pos, SubresourceData((void*)positions.data()));
	m_dev->create_buffer(BufferDesc::vertex(uvs.size() * sizeof(uvs[0])), &uv, SubresourceData((void*)uvs.data()));
	m_dev->create_buffer(BufferDesc::vertex(normals.size() * sizeof(normals[0])), &nor, SubresourceData((void*)normals.data()));
	m_dev->create_buffer(BufferDesc::index(indices.size() * sizeof(indices[0])), &idx, SubresourceData((void*)indices.data()));

	std::vector<std::pair<GPUBuffer, UINT>> vbs_and_strides;
	vbs_and_strides.push_back({ pos, (UINT)sizeof(positions[0]) });
	vbs_and_strides.push_back({ uv, (UINT)sizeof(uvs[0]) });
	vbs_and_strides.push_back({ nor, (UINT)sizeof(normals[0]) });

	// Set partial geometry data for model
	auto model = Model().set_ib(idx).set_vbs(vbs_and_strides);

	for (int i = 0; i < meshes.size(); ++i)
	{
		// Add submesh data
		Mesh mesh;
		const auto& assimp_mesh = meshes[i];
		std::memcpy(&mesh, &assimp_mesh, sizeof(AssimpMeshData));

		// Get material
		const auto& assimp_mat = mats[i];
		//auto mat = load_material(assimp_mat);
		auto mat = m_mat_mgr->load_material(assimp_mat);

		// Add mesh/material pair
		model.add_mesh(mesh, mat);
	}

	std::string model_name = name;
	if (model_name.empty())
		model_name = "Model" + std::to_string(m_def_counter++);

	m_path_mapper.insert({ path, model_name });
	auto ret_it = m_models.insert({ model_name, model });
	return &(ret_it.first->second);
}

const Model* ModelManager::get_model(const std::string& name)
{
	return &(m_models.find(name)->second);
}

void ModelManager::remove_model(const std::string& name)
{
	// Remove model
	m_models.erase(name);

	// Remove path
	std::map<std::filesystem::path, std::string>::iterator del_it;
	for (auto it = m_path_mapper.begin(); it != m_path_mapper.end(); ++it)
		if (it->second == name)
			del_it = it;
	if (del_it != m_path_mapper.end())
		m_path_mapper.erase(del_it);
}
