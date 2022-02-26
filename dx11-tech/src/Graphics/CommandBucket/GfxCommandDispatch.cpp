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

    /* grab md */
    auto aux = gfxcommandpacket::get_aux_memory(cmd);
    assert(aux != nullptr);

    bindtable::Header* md = reinterpret_cast<bindtable::Header*>(aux);
    assert(md->validated == 1);

    //std::cout << "vbs: " << md->vbs << "\n";
    //std::cout << "cbs: " << md->cbs << "\n";
    //std::cout << "samplers: " << md->samplers << "\n";
    //std::cout << "textures: " << md->textures << "\n";

    gfx::dev->bind_pipeline(cmd->pipeline);

    char* payload_now = (char*)md + sizeof(bindtable::Header);
    /*
        Read payload
    */
    auto* vbs = reinterpret_cast<bindtable::PayloadVB*>(payload_now);
    //static std::vector<std::tuple<BufferHandle, UINT, UINT>> vb_binds;
    //vb_binds.resize(md->vbs);
    //for (int i = 0; i < md->vbs; ++i)
    //{
    //    //const auto& vb_dat = vbs[i];
    //    vb_binds[i] = { BufferHandle{vbs[i].hdl}, vbs[i].stride, vbs[i].offset };
    //    //std::cout << "vb: " << vb_dat.hdl << " (hdl) " << vb_dat.stride << " (stride) " << vb_dat.offset << " (offset) \n";
    //}
    //gfx::dev->bind_vertex_buffers(0, vb_binds);
    gfx::dev->bind_vertex_buffers(0, vbs, md->vbs);
    payload_now += md->vbs * sizeof(bindtable::PayloadVB);  // go to next type


    auto* cbs = reinterpret_cast<bindtable::PayloadCB*>(payload_now);
    for (int i = 0; i < md->cbs; ++i)
    {
        const auto& cb_dat = cbs[i];
        //std::cout << "cb: " << cb_dat.hdl << " (hdl) " << (uint64_t)cb_dat.stage << " (stage) " << (uint64_t)cb_dat.slot << " (slot) \n";
        gfx::dev->bind_constant_buffer(cb_dat.slot, (ShaderStage)cb_dat.stage, BufferHandle{ cb_dat.hdl });
    }
    payload_now += md->cbs * sizeof(bindtable::PayloadCB);  // go to next type


    auto* textures = reinterpret_cast<bindtable::PayloadTexture*>(payload_now);
    for (int i = 0; i < md->textures; ++i)
    {
        const auto& texture_dat = textures[i];
        gfx::dev->bind_resource(texture_dat.slot, (ShaderStage)texture_dat.stage, TextureHandle{ texture_dat.hdl });
        //std::cout << "texture: " << texture_dat.hdl << " (hdl) " << (uint64_t)texture_dat.stage << " (stage) " << (uint64_t)texture_dat.slot << " (slot) \n";
    }
    payload_now += md->textures * sizeof(bindtable::PayloadTexture);  // go to next type

    auto* samplers = reinterpret_cast<bindtable::PayloadSampler*>(payload_now);
    for (int i = 0; i < md->samplers; ++i)
    {
        const auto& sampler_dat = samplers[i];
        //std::cout << "sampler: " << sampler_dat.hdl << " (hdl) " << (uint64_t)sampler_dat.stage << " (stage) " << (uint64_t)sampler_dat.slot << " (slot) \n";
        gfx::dev->bind_sampler(sampler_dat.slot, (ShaderStage)sampler_dat.stage, SamplerHandle{ sampler_dat.hdl });
    }

    gfx::dev->bind_index_buffer(cmd->ib);
    gfx::dev->draw_indexed(cmd->index_count, cmd->index_start, cmd->vertex_start);

    //std::cout << "\n\n\n";
}

void gfxcommand_dispatch::copy_to_cbuffer(const void* data)
{



}
