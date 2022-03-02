#include "pch.h"
#include "Profiler/FrameProfiler.h"
#include "Graphics/API/GfxDevice.h"
#include "Graphics/Renderer/Renderer.h"
#include "Graphics/Renderer/ModelRenderer.h"
#include "Graphics/ModelManager.h"

#include "Graphics/CommandBucket/GfxCommand.h"
//#include "Graphics/CommandBucket/GfxCommandPacket.h"


namespace gfx 
{ 
	extern GfxDevice* dev;
	extern ModelManager* model_mgr; 
}

namespace perf
{
	extern CPUProfiler* cpu_profiler;
}

ModelHandle ModelRenderer::load_model(const std::filesystem::path& rel_path)
{
	//return ModelHandle();

	auto p = m_loaded_models.get_next_free_handle();
	ModelHandle hdl;
	hdl.hdl = p.first;
	//p.second->handle = p.first;

	// Load model (temp)
	auto mod = gfx::model_mgr->load_model(rel_path, "Some_Model" + std::to_string(m_counter++));

	// Temporarily copy to our memory
	//std::memcpy(p.second->data, mod, sizeof(Model));
	p.second->data = mod;

	return hdl;
}

void ModelRenderer::free_model(ModelHandle hdl)
{
	m_loaded_models.free_handle(hdl.hdl);

	// Add manual removal later
}


ModelRenderer::ModelRenderer(Renderer* master_renderer) :
	m_master_renderer(master_renderer),
	m_shared_resources(master_renderer->get_shared_resources()),
	m_per_object_data_allocator(make_unique<LinearAllocator>(MAX_SUBMISSION_PER_FRAME * sizeof(PerObjectData)))
{
	m_per_object_cb = gfx::dev->create_buffer(BufferDesc::constant(gfxconstants::MIN_CB_SIZE_FOR_RANGES * MAX_SUBMISSION_PER_FRAME));
}

void ModelRenderer::begin()
{
	m_per_object_data = (PerObjectData*)m_per_object_data_allocator->allocate(MAX_SUBMISSION_PER_FRAME * sizeof(PerObjectData));
}

void ModelRenderer::end()
{
	// Send copy command to GPU
	auto copy_bucket = m_master_renderer->get_copy_bucket();
	auto big_copy = copy_bucket->add_command<gfxcommand::CopyToBuffer>(0, 0);
	big_copy->buffer = m_per_object_cb;
	big_copy->data = m_per_object_data;
	big_copy->data_size = m_submission_count * sizeof(PerObjectData);

	m_submission_count = 0;
	m_per_object_data_allocator->reset();

}

void ModelRenderer::submit(ModelHandle hdl, const DirectX::SimpleMath::Matrix& wm, ModelRenderSpec spec)
{
	auto _ = CPUProfiler::ScopedAccum("Model Submission");

	const auto& model = m_loaded_models.look_up(hdl.hdl)->data;
	const auto& meshes = model->get_meshes();
	const auto& materials = model->get_materials();

	// Store world matrix for this submission
	m_per_object_data[m_submission_count].world_mat = wm;

	auto opaque_bucket = m_master_renderer->get_opaque_bucket();
	auto transp_bucket = m_master_renderer->get_transparent_bucket();
	auto shadow_bucket = m_master_renderer->get_shadow_bucket();

	for (int i = 0; i < meshes.size(); ++i)
	{
		const auto& mesh = meshes[i];
		const auto& mat = materials[i];

		uint8_t vbs_in = (uint8_t)model->get_vb().size();
		uint8_t cbs_in = 1;	
		uint8_t textures_in = 1;
		uint8_t samplers_in = 0;

		// Key based on texture
		uint64_t key = mat->get_texture(Material::Texture::eAlbedo).hdl;

		// Create draw call
		size_t aux_size = gfxcommand::aux::bindtable::get_size(vbs_in, cbs_in, textures_in, samplers_in);
		auto cmd = opaque_bucket->add_command<gfxcommand::Draw>(key, aux_size);
		cmd->ib = model->get_ib();
		cmd->index_count = mesh.index_count;
		cmd->index_start = mesh.index_start;
		cmd->vertex_start = mesh.vertex_start;
		cmd->pipeline = m_shared_resources->deferred_gpass_pipe;	// Should come from material if Forward or internal GPass pipeline if Deferred

		// Fill draw call payload (bind table)
		auto payload = gfxcommand::aux::bindtable::Filler(gfxcommandpacket::get_aux_memory(cmd), vbs_in, cbs_in, samplers_in, textures_in);
		for (int i = 0; i < vbs_in; ++i)
			payload.add_vb(std::get<0>(model->get_vb()[i]).hdl, std::get<1>(model->get_vb()[i]), std::get<2>(model->get_vb()[i]));

		payload.add_cb(m_per_object_cb.hdl, (uint8_t)ShaderStage::eVertex, 1, m_submission_count);
		payload.add_texture(mat->get_texture(Material::Texture::eAlbedo).hdl, (uint8_t)ShaderStage::ePixel, 0);
		payload.validate();
			
		// Replicate draw for shadow, but only using positions
		if (spec.casts_shadow)
		{
			vbs_in = 1;
			cbs_in = 1;
			textures_in = 0;
			samplers_in = 0;

			// Key based on distance (should be in viewspace ..)
			key = 1.f - (wm.Translation().z / std::numeric_limits<float>::max());

			// Create draw call
			aux_size = gfxcommand::aux::bindtable::get_size(vbs_in, cbs_in, textures_in, samplers_in);
			cmd = shadow_bucket->add_command<gfxcommand::Draw>(key, aux_size);
			cmd->ib = model->get_ib();
			cmd->index_count = mesh.index_count;
			cmd->index_start = mesh.index_start;
			cmd->vertex_start = mesh.vertex_start;
			cmd->pipeline = m_shared_resources->depth_only_pipe;	// Always uses depth only pipe for shadow rendering

			// Fill draw call payload (bind table)
			payload = gfxcommand::aux::bindtable::Filler(gfxcommandpacket::get_aux_memory(cmd), vbs_in, cbs_in, samplers_in, textures_in);
			for (int i = 0; i < vbs_in; ++i)
				payload.add_vb(std::get<0>(model->get_vb()[i]).hdl, std::get<1>(model->get_vb()[i]), std::get<2>(model->get_vb()[i]));

			payload.add_cb(m_per_object_cb.hdl, (uint8_t)ShaderStage::eVertex, 1, m_submission_count);
			payload.validate();
		}
	}

	++m_submission_count;
}
