#include "pch.h"
#include "AssimpLoader.h"


using namespace DirectX::SimpleMath;

AssimpLoader::AssimpLoader(const std::filesystem::path& fpath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		fpath.relative_path().string().c_str(),
		aiProcess_Triangulate |

		// For Direct3D
		aiProcess_ConvertToLeftHanded |
		//aiProcess_FlipUVs |					// (0, 0) is top left
		//aiProcess_FlipWindingOrder |			// D3D front face is CW

		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |

		// Extra flags (http://assimp.sourceforge.net/lib_html/postprocess_8h.html#a64795260b95f5a4b3f3dc1be4f52e410a444a6c9d8b63e6dc9e1e2e1edd3cbcd4)
		aiProcess_JoinIdenticalVertices |
		aiProcess_ImproveCacheLocality
	);

	if (!scene)
	{
		fmt::print("File not found!\n");
		return;
	}

	/*
		Find out total amount of vertices and pre-allocate memory
	*/
	unsigned int total_verts = 0;
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		total_verts += scene->mMeshes[i]->mNumVertices;

	m_positions.reserve(total_verts);
	m_uvs.reserve(total_verts);
	m_normals.reserve(total_verts);
	m_indices.reserve(total_verts);
	m_tangents.reserve(total_verts);
	m_bitangents.reserve(total_verts);
	m_meshes.reserve(scene->mNumMeshes); 


	// start processing scene
	process_node(scene->mRootNode, scene);
}

void AssimpLoader::process_mesh(aiMesh* mesh, const aiScene* scene)
{
	/*
		Get all the the relevant vertex data from this mesh
	*/
	UINT vertex_start = m_positions.size();
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		Vector3 pos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		Vector3 nor(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

		m_positions.push_back(pos);
		m_normals.push_back(nor);

		if (mesh->mTextureCoords[0])
		{
			Vector2 uv(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			m_uvs.push_back(uv);
		}

		if (mesh->mTangents)
		{
			Vector3 tangent(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			m_tangents.push_back(tangent);
		}

		if (mesh->mBitangents)
		{
			Vector3 bitangent(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
			m_bitangents.push_back(bitangent);
		}
	}
	

	/*
		Save where this mesh starts in the index buffer
	*/
	//size_t index_start = (std::max)((int64_t)m_indices.size() - 1, (int64_t)0);
	auto index_start = m_indices.size();

	/*
		Go over this meshes faces and extract indices.
		If triangulation is enabled, each face should have 3 vertices.
	*/
	unsigned int index_count = 0;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			m_indices.push_back(face.mIndices[j]);
			++index_count;
		}
	}
	
	AssimpMeshData amd;
	amd.index_start = index_start;
	amd.index_count = index_count;

	/*
		It seems like the indices are local to the mesh part, hence why vertex_start is required.
		
	*/
	amd.vertex_start = vertex_start;
	m_meshes.push_back(amd);

	
}

void AssimpLoader::process_node(aiNode* node, const aiScene* scene)
{
	/*
		Process all meshes in this node
	*/
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		process_mesh(mesh, scene);
	}

	/*
		Recursively process all child nodes and process meshes in them.
	*/
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		process_node(node->mChildren[i], scene);

}
