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

const std::vector<GPUBuffer>& Model::get_vbs() const
{
	return m_vbs;
}

const GPUBuffer* Model::get_ib() const
{
	return &m_ib;
}

const std::vector<Mesh>& Model::get_meshes() const
{
	return m_meshes;
}

const std::vector<const Material*>& Model::get_materials() const
{
	return m_materials;
}
