#include "pch.h"
#include "Graphics/Model.h"

Model& Model::set_vbs(const std::vector<GPUBuffer> vbs)
{
	m_vbs = vbs;
	return *this;
}

Model& Model::set_ib(GPUBuffer ib)
{
	m_ib = ib;
	return *this;
}

Model& Model::add_mesh(Mesh mesh, const Material* mat)
{
	m_meshes.push_back(mesh);
	m_materials.push_back(mat);
	return *this;
}

const std::vector<GPUBuffer>& Model::get_vbs()
{
	return m_vbs;
}

GPUBuffer* Model::get_ib()
{
	return &m_ib;
}

const std::vector<Mesh>& Model::get_meshes()
{
	return m_meshes;
}

const std::vector<const Material*>& Model::get_materials()
{
	return m_materials;
}
