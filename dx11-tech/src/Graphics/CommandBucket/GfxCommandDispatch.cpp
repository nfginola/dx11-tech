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

void gfxcommand_dispatch::copy_to_buffer(const void* data)
{
    const gfxcommand::CopyToBuffer* cmd = reinterpret_cast<const gfxcommand::CopyToBuffer*>(data);
    gfx::dev->map_copy(cmd->buffer, SubresourceData(cmd->data, (UINT)cmd->data_size));
}
