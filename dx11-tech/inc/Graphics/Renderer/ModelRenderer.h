#pragma once
#include "ResourceHandlePool.h"
#include "Graphics/API/GfxHandles.h"
#include "Graphics/API/GfxCommon.h"
#include "Graphics/Model.h"
#include "Memory/Allocator.h"

class Renderer;
struct RendererSharedResources;

struct ModelHandle { res_handle hdl; };

struct ModelRenderSpec
{
	bool casts_shadow = true;
};

/*
	System responsible for models which owns models and renders them.
*/
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
	
	void submit(ModelHandle hdl, const DirectX::SimpleMath::Matrix& mat, ModelRenderSpec spec = {});


private:
	Renderer* m_master_renderer;
	const RendererSharedResources* m_shared_resources;


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
	struct alignas(gfxconstants::MIN_CB_SIZE_FOR_RANGES) PerObjectData
	{
		DirectX::XMMATRIX world_mat;
	};
	BufferHandle m_per_object_cb;

	// Max model submissions per frame
	static constexpr UINT MAX_SUBMISSION_PER_FRAME = 1000;

	unique_ptr<Allocator> m_per_object_data_allocator;
	PerObjectData* m_per_object_data = nullptr;
	uint32_t m_submission_count = 0;

	// Temporary (should be removed later)
	//PipelineHandle m_def_pipeline;

	// should be moved 
	//PipelineHandle m_depth_only_pipe;


};

