#pragma once
#include "Graphics/API/GfxTypes.h"
#include "Graphics/Material.h"

struct Mesh
{
	UINT index_start = 0;
	UINT index_count = 0;
	UINT vertex_start = 0;
};

class Model
{
public:
	Model() = default;
	~Model() = default;

	Model& set_vbs(const std::vector<std::pair<GPUBuffer, UINT>>& vbs_and_strides);
	Model& set_ib(GPUBuffer ib);
	Model& add_mesh(Mesh mesh, const Material* mat);

	const std::vector<GPUBuffer>& get_vbs() const;
	const std::vector<UINT>& get_vb_strides() const;
	const GPUBuffer* get_ib() const;
	
	const std::vector<Mesh>& get_meshes() const;
	const std::vector<const Material*>& get_materials() const;

private:
	std::vector<GPUBuffer> m_vbs;
	std::vector<UINT> m_vb_strides;
	GPUBuffer m_ib;

	std::vector<Mesh> m_meshes;
	std::vector<const Material*> m_materials;
};
