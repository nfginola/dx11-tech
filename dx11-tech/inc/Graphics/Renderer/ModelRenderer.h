#pragma once
#include "ResourceHandlePool.h"
#include "Graphics/API/GfxHandles.h"
#include "Graphics/Model.h"

class Renderer;

struct ModelHandle { res_handle hdl; };

struct ModelRenderSpec
{
	bool is_transparent = false;
	bool casts_shadow = true;
};

class ModelRenderer
{
public:
	ModelRenderer() = delete;
	ModelRenderer(Renderer* master_renderer);
	~ModelRenderer() {};

	void begin();
	void end();

	ModelHandle load_model(const std::filesystem::path& rel_path);
	void free_model(ModelHandle hdl);
	
	void submit(ModelHandle hdl, const DirectX::SimpleMath::Matrix& mat, ModelRenderSpec spec);


private:
	Renderer* m_master_renderer;

	struct ModelInternal
	{
		res_handle handle;
		const Model* data;			// temporarily a pointer

		void free() {};
	};

	ResourceHandlePool<ModelInternal> m_loaded_models;
	uint64_t m_counter = 0;

private:
	// Per Object data
	struct alignas(256) CBPerObject
	{
		DirectX::XMMATRIX world_mat;
	};
	std::array<CBPerObject, 5> m_per_object_data;
	BufferHandle m_per_object_cb;

	uint32_t m_submission_count = 0;

	PipelineHandle m_def_pipeline;
	

};

