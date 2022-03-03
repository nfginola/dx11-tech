#include "pch.h"
#include "Graphics/CommandBucket/GfxCommandDispatch.h"
#include "Graphics/CommandBucket/GfxCommand.h"
#include "Graphics/CommandBucket/GfxCommandPacket.h"
#include "Graphics/API/GfxDevice.h"

// Global dependencies
namespace gfx
{
	extern GfxDevice* dev;
}

void gfxcommand_dispatch::draw(const void* data)
{
    using namespace gfxcommand::aux;
    const gfxcommand::Draw* cmd = reinterpret_cast<const gfxcommand::Draw*>(data);

    // Grab bind table
    auto aux = gfxcommandpacket::get_aux_memory(cmd);
    assert(aux != nullptr);
    bindtable::Header* md = reinterpret_cast<bindtable::Header*>(aux);
    assert(md->validated == 1);
    
    // Grab payload
    char* payload_now = (char*)md + sizeof(bindtable::Header);

    // Bind VBs
    auto* vbs = reinterpret_cast<bindtable::PayloadVB*>(payload_now);
    gfx::dev->bind_vertex_buffers(0, vbs, md->vbs);
    payload_now += md->vbs * sizeof(bindtable::PayloadVB);  // go to next type

    // Bind CBs
    auto* cbs = reinterpret_cast<bindtable::PayloadCB*>(payload_now);
    for (unsigned int i = 0; i < md->cbs; ++i)
    {
        const auto& cb_dat = cbs[i];
        gfx::dev->bind_constant_buffer(cb_dat.slot, (ShaderStage)cb_dat.stage, BufferHandle{ cb_dat.hdl }, cb_dat.offset56s, cb_dat.range56s);
    }
    payload_now += md->cbs * sizeof(bindtable::PayloadCB);  // go to next type
    
    // Bind read textures
    auto* textures = reinterpret_cast<bindtable::PayloadTexture*>(payload_now);
    for (unsigned int i = 0; i < md->textures; ++i)
    {
        const auto& texture_dat = textures[i];
        gfx::dev->bind_resource(texture_dat.slot, (ShaderStage)texture_dat.stage, TextureHandle{ texture_dat.hdl });
    }
    payload_now += md->textures * sizeof(bindtable::PayloadTexture);  // go to next type
    
    // Bind samplers
    auto* samplers = reinterpret_cast<bindtable::PayloadSampler*>(payload_now);
    for (unsigned int i = 0; i < md->samplers; ++i)
    {
        const auto& sampler_dat = samplers[i];
        gfx::dev->bind_sampler(sampler_dat.slot, (ShaderStage)sampler_dat.stage, SamplerHandle{ sampler_dat.hdl });
    }
    
    // Draw
    gfx::dev->bind_pipeline(cmd->pipeline);
    gfx::dev->bind_index_buffer(cmd->ib);
    gfx::dev->draw_indexed(cmd->index_count, cmd->index_start, cmd->vertex_start);

}

void gfxcommand_dispatch::draw2(const void* data)
{
	const gfxcommand::Draw* cmd = reinterpret_cast<const gfxcommand::Draw*>(data);
	using namespace gfxcommand::aux::computebindtable;

	// Grab bind table
	auto aux = gfxcommandpacket::get_aux_memory(cmd);
	ComputeHeader* hdr = (ComputeHeader*)aux;
	const auto& counts = hdr->get_counts();
	char* payload = (char*)hdr + sizeof(ComputeHeader);
	char* look_now = payload;

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

	// Draw
	gfx::dev->bind_pipeline(cmd->pipeline);
	gfx::dev->bind_index_buffer(cmd->ib);
	gfx::dev->draw_indexed(cmd->index_count, cmd->index_start, cmd->vertex_start);
}


void gfxcommand_dispatch::copy_to_buffer(const void* data)
{
    const gfxcommand::CopyToBuffer* cmd = reinterpret_cast<const gfxcommand::CopyToBuffer*>(data);
    gfx::dev->map_copy(cmd->buffer, SubresourceData(cmd->data, (UINT)cmd->data_size));
}
