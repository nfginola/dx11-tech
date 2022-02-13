#include "pch.h"
#include "Graphics/Model.h"

Model& Model::set_vbs(const std::vector<std::tuple<BufferHandle, UINT, UINT>>& vbs_strides_offsets)
{
	m_vbs_strides_offsets = vbs_strides_offsets;
	//for (const auto& p : vbs_and_strides)
	//{
	//	m_vbs.push_back(p.first);
	//	m_vb_strides.push_back(p.second);
	//}

	return *this;
}

Model& Model::set_ib(BufferHandle ib)
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

//const std::vector<GPUBuffer>& Model::get_vbs() const
//{
//	return m_vbs;
//}
//
//const std::vector<UINT>& Model::get_vb_strides() const
//{
//	return m_vb_strides;
//}
//
//const GPUBuffer* Model::get_ib() const
//{
//	return &m_ib;
//}

const std::vector<std::tuple<BufferHandle, UINT, UINT>>& Model::get_vb() const
{
	return m_vbs_strides_offsets;
}

BufferHandle Model::get_ib() const
{
	return m_ib;
}

const std::vector<Mesh>& Model::get_meshes() const
{
	return m_meshes;
}

const std::vector<const Material*>& Model::get_materials() const
{
	return m_materials;
}
