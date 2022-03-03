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

	ShaderHandle cs = gfx::dev->compile_and_create_shader(ShaderStage::eCompute, "ComputeShader.hlsl");
	m_compute_pipe = gfx::dev->create_compute_pipeline(ComputePipelineDesc(ComputeShader(cs)));
}

void ModelRenderer::begin()
{
	m_per_object_data = (PerObjectData*)m_per_object_data_allocator->allocate(MAX_SUBMISSION_PER_FRAME * sizeof(PerObjectData));
}

static bool done = false;

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

	done = false;
}

void ModelRenderer::submit(ModelHandle hdl, const DirectX::SimpleMath::Matrix& wm, ModelRenderSpec spec)
{
	auto _ = FrameProfiler::ScopedCPUAccum("Model Submission");

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

		uint64_t key = mat->get_texture(Material::Texture::eAlbedo).hdl;
		
		// .. Setup header for payload ..
		auto hdr = gfxcommand::aux::bindtable::Header()
			.set_vbs((uint8_t)model->get_vb().size())
			.set_cbs(1)
			.set_tex_reads(1);

		// .. and allocate command ..
		auto cmd = opaque_bucket->add_command<gfxcommand::Draw>(key, hdr.size());
		cmd->ib = model->get_ib();
		cmd->index_count = mesh.index_count;
		cmd->index_start = mesh.index_start;
		cmd->vertex_start = mesh.vertex_start;
		cmd->pipeline = m_shared_resources->deferred_gpass_pipe;

		// .. and fill binding table
		auto payload = gfxcommand::aux::bindtable::Filler(gfxcommandpacket::get_aux_memory(cmd), hdr);
		for (int i = 0; i < model->get_vb().size(); ++i)
			payload.add_vb(std::get<0>(model->get_vb()[i]), std::get<1>(model->get_vb()[i]), std::get<2>(model->get_vb()[i]));
		payload
			.add_cb(ShaderStage::eVertex, 1, m_per_object_cb, m_submission_count)
			.add_read_tex(ShaderStage::ePixel, 0, mat->get_texture(Material::Texture::eAlbedo));


		// Replicate draw for shadow, but only using positions
		if (spec.casts_shadow)
		{
			auto shadow_hdr = gfxcommand::aux::bindtable::Header()
				.set_vbs(1)	// position only
				.set_cbs(1);
			
			auto shadow_cmd = shadow_bucket->add_command<gfxcommand::Draw>(0, shadow_hdr.size());
			shadow_cmd->ib = model->get_ib();
			shadow_cmd->index_count = mesh.index_count;
			shadow_cmd->index_start = mesh.index_start;
			shadow_cmd->vertex_start = mesh.vertex_start;
			shadow_cmd->pipeline = m_shared_resources->depth_only_pipe;	// Always uses depth only pipe for shadow rendering

			gfxcommand::aux::bindtable::Filler(gfxcommandpacket::get_aux_memory(shadow_cmd), shadow_hdr)
				.add_vb(std::get<0>(model->get_vb()[0]), std::get<1>(model->get_vb()[0]), std::get<2>(model->get_vb()[0]))
				.add_cb(ShaderStage::eVertex, 1, m_per_object_cb, m_submission_count);
		}
	}


	if (!done)
	{
		auto compute_bucket = m_master_renderer->get_compute_bucket();

		// Create command with zero-initialized header
		auto hdr = gfxcommand::aux::bindtable::Header();
		auto compute_cmd = compute_bucket->add_command<gfxcommand::ComputeDispatch>(0, hdr.size());
		auto zero_init = gfxcommand::aux::bindtable::Filler(gfxcommandpacket::get_aux_memory(compute_cmd), hdr);

		compute_cmd->pipeline = m_compute_pipe;
		compute_cmd->x_blocks = 3;
		compute_cmd->y_blocks = 4;
		compute_cmd->z_blocks = 5;
		compute_cmd->set_profile_name("Null Compute");
		done = true;
	}


	++m_submission_count;
}