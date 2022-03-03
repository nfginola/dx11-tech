#include "pch.h"
#include "Graphics/CommandBucket/GfxCommandDispatch.h"
#include "Graphics/CommandBucket/GfxCommand.h"
#include "Graphics/CommandBucket/GfxCommandPacket.h"
#include "Graphics/API/GfxDevice.h"
#include "Profiler/FrameProfiler.h"

// Global dependencies
namespace gfx
{
	extern GfxDevice* dev;
}
// FrameProfiler

// ================== helper decls
void bind_resource_table(char* look_now, const gfxcommand::aux::bindtable::ResourceCounts& counts);


// ================== commands

void gfxcommand_dispatch::draw(const void* data)
{
	const auto cmd = (const gfxcommand::Draw*)data;
	using namespace gfxcommand::aux::bindtable;

	// Grab bind table header and payload
	auto aux = gfxcommandpacket::get_aux_memory(cmd);
	Header* hdr = (Header*)aux;
	const auto& counts = hdr->get_counts();
	char* payload = (char*)hdr + sizeof(Header);
	char* look_now = payload;
	
	// Bind resources
	bind_resource_table(look_now, counts);

	// Draw
	gfx::dev->bind_pipeline(cmd->pipeline);
	gfx::dev->bind_index_buffer(cmd->ib);
	gfx::dev->draw_indexed(cmd->index_count, cmd->index_start, cmd->vertex_start);
}

void gfxcommand_dispatch::dispatch(const void* data)
{
	const auto cmd = (const gfxcommand::ComputeDispatch*)data;
	using namespace gfxcommand::aux::bindtable;

	// Grab bind table header and payload
	auto aux = gfxcommandpacket::get_aux_memory(cmd);
	Header* hdr = (Header*)aux;
	const auto& counts = hdr->get_counts();
	char* payload = (char*)hdr + sizeof(Header);
	char* look_now = payload;

	// Bind resources (user is responsible for making sure the binds are directed to the CS stage)
	// Otherwise, binds on the normal rendering pipeline are done (if any)
	//bind_resource_table(look_now, counts);

	// Bind compute and dispatch
	if (cmd->profile_name[0] == '\0')
	{
		gfx::dev->dispatch(cmd->x_blocks, cmd->y_blocks, cmd->z_blocks);
		bind_resource_table(look_now, counts);

	}
	else
	{
		auto _ = FrameProfiler::Scoped(cmd->profile_name.data());
		// Is read/write stalls recorded by scoping over binds? (I will assume yes for now)
		bind_resource_table(look_now, counts);							
		gfx::dev->dispatch(cmd->x_blocks, cmd->y_blocks, cmd->z_blocks);
	}
}

void gfxcommand_dispatch::copy_to_buffer(const void* data)
{
    const gfxcommand::CopyToBuffer* cmd = reinterpret_cast<const gfxcommand::CopyToBuffer*>(data);
    gfx::dev->map_copy(cmd->buffer, SubresourceData(cmd->data, (UINT)cmd->data_size));
}



// ================== helper defs
void bind_resource_table(char* look_now, const gfxcommand::aux::bindtable::ResourceCounts& counts)
{
	/*
		Payload layout:
			PayloadVB
			..
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
	
	using namespace gfxcommand::aux::bindtable;

	// Bind VBs
	gfx::dev->bind_vertex_buffers(0, look_now, counts.vbs);
	look_now += counts.vbs * sizeof(PayloadVB);

	for (uint64_t i = 0; i < counts.cbs; ++i)
	{
		auto cb = (const PayloadCB* const)look_now;
		gfx::dev->bind_constant_buffer(cb->slot, (ShaderStage)cb->stage, cb->hdl, cb->offset56s, cb->range56s);
		look_now += sizeof(PayloadCB);
	}

	for (uint64_t i = 0; i < counts.tex_reads; ++i)
	{
		auto tex = (const PayloadTexture* const)look_now;
		gfx::dev->bind_resource(tex->slot, (ShaderStage)tex->stage, tex->hdl);
		look_now += sizeof(PayloadTexture);
	}

	for (uint64_t i = 0; i < counts.buf_reads; ++i)
	{
		auto buffer = (const PayloadBuffer* const)look_now;
		gfx::dev->bind_resource(buffer->slot, (ShaderStage)buffer->stage, buffer->hdl);
		look_now += sizeof(PayloadBuffer);
	}

	// initial count hardcoded for submissions (fix later if needed)
	static constexpr UINT initial_count = 0;
	for (uint64_t i = 0; i < counts.tex_rws; ++i)
	{
		auto tex = (const PayloadTexture* const)look_now;
		gfx::dev->bind_resource_rw(tex->slot, (ShaderStage)tex->stage, tex->hdl, initial_count);
		look_now += sizeof(PayloadTexture);
	}

	for (uint64_t i = 0; i < counts.buf_rws; ++i)
	{
		auto buffer = (const PayloadBuffer* const)look_now;
		gfx::dev->bind_resource_rw(buffer->slot, (ShaderStage)buffer->stage, buffer->hdl, initial_count);
		look_now += sizeof(PayloadBuffer);
	}

	for (uint64_t i = 0; i < counts.samplers; ++i)
	{
		auto sampler = (const PayloadSampler* const)look_now;
		gfx::dev->bind_sampler(sampler->slot, (ShaderStage)sampler->stage, sampler->hdl);
		look_now += sizeof(PayloadCB);
	}
}
