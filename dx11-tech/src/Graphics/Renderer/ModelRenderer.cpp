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

	int i = sizeof(gfxcommand::aux::computebindtable::ComputeHeader);
	int x = sizeof(gfxcommand::aux::computebindtable::ComputeFiller);
	int y = sizeof(gfxcommand::aux::computebindtable::PayloadBuffer);
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

void thingy(void* cmd);

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

		payload.add_cb(m_per_object_cb.hdl, ShaderStage::eVertex, 1, m_submission_count);
		payload.add_texture(mat->get_texture(Material::Texture::eAlbedo).hdl, ShaderStage::ePixel, 0);
		payload.validate();
			
		// Replicate draw for shadow, but only using positions
		if (spec.casts_shadow)
		{
			vbs_in = 1;
			cbs_in = 1;
			textures_in = 0;
			samplers_in = 0;

			// Key based on distance (should be in viewspace ..)
			uint16_t new_key = (uint16_t)(1.f - (wm.Translation().z / std::numeric_limits<float>::max()));

			// Create draw call
			aux_size = gfxcommand::aux::bindtable::get_size(vbs_in, cbs_in, textures_in, samplers_in);
			cmd = shadow_bucket->add_command<gfxcommand::Draw>(new_key, aux_size);
			cmd->ib = model->get_ib();
			cmd->index_count = mesh.index_count;
			cmd->index_start = mesh.index_start;
			cmd->vertex_start = mesh.vertex_start;
			cmd->pipeline = m_shared_resources->depth_only_pipe;	// Always uses depth only pipe for shadow rendering

			/*
				// Declare header
				// Build pattern lets you write only what you use (decrease bloat)
				auto header = gfxcommand::aux::computebindtable::ComputeHeader()
					.set_cbs(3)
					.set_tex_reads(2)
					.set_buf_reads(1)
					.set_tex_rws(4)
					.set_buf_rws(1)
					.set_samplers(1);

				add_command(aux, header.get_size())		// get_size returns sizeof(Header) + all require bindings size

				
				
				// constructor should verify that the declared binds in the header is less in size than the payload
				// think of this as a deferred validation, since Header and Payload are cohesive

				// Constructor copies Header into the beginning of aux and then sets the payload..
				auto payload = gfxcommand::aux::computebindtable::ComputeFiller(aux, header)
					.set_cb(..., 0)
					.set_cb(..., 1)
					...
					
					only validation needed is during set where we check if curr_bound < max_bound!
						--> less room for error (e.g forgetting to validate at the end)
					
					support arbitrary order of setting payload by reserving exactly the supplied amount of space per bind attribute
					[header, cb, cb, cb, tex, tex, buf, tex, tex, tex, tex, buf, sampler] (using header above as example for this load)
					
					// note that this is a part of Filler (helper) and not a part of the Payload! :)
					// uint16_t for below types should be enough
					cb_offset = sizeof(Header)
					tex_read_offset = cb_offset + cbs * sizeof(PayloadCB)
					buf_read_offset = tex_read_offset + tex_reads * sizeof(TexturePayload)
					tex_rws_offset = buf_read_offset + buf_reads * sizeof(BufferPayload)
					buf_rws_offset = tex_rws_offset + tex_rws * sizeof(TexturePayload)
					samplers_offset = buf_rws_offset + buf_rws * sizeoof(BufferPayload)

			*/

			// Fill draw call payload (bind table)
			payload = gfxcommand::aux::bindtable::Filler(gfxcommandpacket::get_aux_memory(cmd), vbs_in, cbs_in, samplers_in, textures_in);
			for (int i = 0; i < vbs_in; ++i)
				payload.add_vb(std::get<0>(model->get_vb()[i]).hdl, std::get<1>(model->get_vb()[i]), std::get<2>(model->get_vb()[i]));

			payload.add_cb(m_per_object_cb.hdl, ShaderStage::eVertex, 1, m_submission_count);
			payload.validate();
		}


		//// Custom call
		//auto hdr = gfxcommand::aux::computebindtable::ComputeHeader()
		//	.set_cbs(3)
		//	.set_buf_reads(1)
		//	.set_tex_reads(2);

		//auto cmd2 = transp_bucket->add_command<gfxcommand::Draw>(0, hdr.size());

		//// fill header + payload in aux memory
		//gfxcommand::aux::computebindtable::ComputeFiller(gfxcommandpacket::get_aux_memory(cmd2), hdr)
		//	.add_cb(BufferHandle{ 1 }, ShaderStage::eVertex, 0)
		//	.add_cb(BufferHandle{ 5 }, ShaderStage::eVertex, 1)
		//	.add_cb(BufferHandle{ 13 }, ShaderStage::eVertex, 2)
		//	.add_read_tex(TextureHandle{ 50 }, ShaderStage::ePixel, 3)
		//	.add_read_buf(BufferHandle{ 31 }, ShaderStage::ePixel, 10)
		//	.add_read_tex(TextureHandle{ 71 }, ShaderStage::ePixel, 1);
		//	
		//thingy(gfxcommandpacket::get_aux_memory(cmd2));


	}

	++m_submission_count;
}

void thingy(void* cmd)
{
	using namespace gfxcommand::aux::computebindtable;

	ComputeHeader* hdr = (ComputeHeader*)cmd;
	const auto& counts = hdr->get_counts();
	char* payload = (char*)hdr + sizeof(ComputeHeader);
	char* look_now = payload;

	/*
	Payload layout:
		PayloadCB
		..
		PayloadTexture (Read)
		..
		PayloadBuffer (Read)
		..
		PayloadTexture (RW)
		..
		PayloadBuffer (RW)
		..
		Sampler
	*/
	for (uint64_t i = 0; i < counts.cbs; ++i)
	{
		auto cb = (PayloadCB*)look_now;
		
		fmt::print("Buffer: {}, {}, {}\n", cb->hdl.hdl, cb->slot, cb->stage);

		look_now += sizeof(PayloadCB);
	}	

	for (uint64_t i = 0; i < counts.tex_reads; ++i)
	{
		auto tex = (PayloadTexture*)look_now;

		fmt::print("Texture: {}, {}, {}\n", tex->hdl.hdl, tex->slot, tex->stage);

		look_now += sizeof(PayloadTexture);
	}

	for (uint64_t i = 0; i < counts.buf_reads; ++i)
	{
		auto buffer = (PayloadBuffer*)look_now;

		fmt::print("Buffer: {}, {}, {}\n", buffer->hdl.hdl, buffer->slot, buffer->stage);

		look_now += sizeof(PayloadBuffer);
	}

	for (uint64_t i = 0; i < counts.tex_rws; ++i)
	{
		auto tex = (PayloadTexture*)look_now;

		fmt::print("Texture: {}, {}, {}\n", tex->hdl.hdl, tex->slot, tex->stage);

		look_now += sizeof(PayloadTexture);
	}

	for (uint64_t i = 0; i < counts.buf_rws; ++i)
	{
		auto buffer = (PayloadBuffer*)look_now;

		fmt::print("Buffer: {}, {}, {}\n", buffer->hdl.hdl, buffer->slot, buffer->stage);

		look_now += sizeof(PayloadBuffer);
	}


	for (uint64_t i = 0; i < counts.samplers; ++i)
	{
		auto sampler = (PayloadSampler*)look_now;

		fmt::print("Sampler: {}, {}, {}\n", sampler->hdl.hdl, sampler->slot, sampler->stage);

		look_now += sizeof(PayloadCB);
	}
}
