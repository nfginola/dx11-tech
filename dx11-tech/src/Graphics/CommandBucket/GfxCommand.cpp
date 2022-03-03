#include "pch.h"
#include "Graphics/CommandBucket/GfxCommand.h"
#include "Graphics/CommandBucket/GfxCommandDispatch.h"

/*
	Assign dispatch functions to commands and define helpers for filling auxiliary memory
*/

/*
    Draw command
*/
const GfxCommandDispatch gfxcommand::Draw::DISPATCH = gfxcommand_dispatch::draw;
const GfxCommandDispatch gfxcommand::ComputeDispatch::DISPATCH = gfxcommand_dispatch::dispatch;
const GfxCommandDispatch gfxcommand::CopyToBuffer::DISPATCH = gfxcommand_dispatch::copy_to_buffer;

namespace gfxcommand::aux::bindtable
{
    // validation
    Filler::~Filler()
    {
        // check so that we have added the bindings that we have declared
        assert(active_counts == hdr.counts);
    }

    Filler::Filler(void* memory, const Header& header) :
        hdr(header),
        payload_start((char*)memory + sizeof(Header))
    {
        // copy header to the start of the memory
        *(Header*)memory = header;
    }

    Filler& Filler::add_vb(BufferHandle handle, uint32_t stride, uint32_t offset)
    {
        assert(active_counts.vbs < hdr.counts.vbs);

        char* vbs_start = payload_start + vb_offset;
        PayloadVB* mem = (PayloadVB*)(vbs_start + active_counts.vbs * sizeof(PayloadVB));
        mem->hdl = handle;
        mem->stride = stride;
        mem->offset = offset;

        ++active_counts.vbs;
        return *this;
    }
    Filler& Filler::add_cb(ShaderStage stage, uint8_t slot, BufferHandle handle, uint32_t offset56s, uint32_t range56s)
    {
        assert(active_counts.cbs < hdr.counts.cbs);

        char* cbs_start = payload_start + cb_offset;
        PayloadCB* mem = (PayloadCB*)(cbs_start + active_counts.cbs * sizeof(PayloadCB));
        mem->hdl = handle;
        mem->stage = (uint8_t)stage;
        mem->slot = slot;
        mem->offset56s = offset56s;
        mem->range56s = range56s;

        ++active_counts.cbs;
        return *this;
    }
    Filler& Filler::add_read_tex(ShaderStage stage, uint8_t slot, TextureHandle handle)
    {
        assert(active_counts.tex_reads < hdr.counts.tex_reads);

        char* tex_reads_start = payload_start + tex_read_offset;
        PayloadTexture* mem = (PayloadTexture*)(tex_reads_start + active_counts.tex_reads * sizeof(PayloadTexture));
        mem->hdl = handle;
        mem->stage = (uint8_t)stage;
        mem->slot = slot;

        ++active_counts.tex_reads;
        return *this;
    }
    Filler& Filler::add_read_buf(ShaderStage stage, uint8_t slot, BufferHandle handle)
    {
        assert(active_counts.buf_reads < hdr.counts.buf_reads);

        char* buf_reads_start = payload_start + buf_read_offset;
        PayloadBuffer* mem = (PayloadBuffer*)(buf_reads_start + active_counts.buf_reads * sizeof(PayloadBuffer));
        mem->hdl = handle;
        mem->stage = (uint8_t)stage;
        mem->slot = slot;

        ++active_counts.buf_reads;
        return *this;
    }
    Filler& Filler::add_rw_tex(ShaderStage stage, uint8_t slot, TextureHandle handle)
    {
        assert(active_counts.tex_rws < hdr.counts.tex_rws);

        char* tex_rws_start = payload_start + tex_rws_offset;
        PayloadTexture* mem = (PayloadTexture*)(tex_rws_start + active_counts.tex_rws * sizeof(PayloadTexture));
        mem->hdl = handle;
        mem->stage = (uint8_t)stage;
        mem->slot = slot;

        ++active_counts.tex_rws;
        return *this;
    }
    Filler& Filler::add_rw_buf(ShaderStage stage, uint8_t slot, BufferHandle handle)
    {
        assert(active_counts.buf_rws < hdr.counts.buf_rws);

        char* buf_rws_start = payload_start + buf_rws_offset;
        PayloadBuffer* mem = (PayloadBuffer*)(buf_rws_start + active_counts.buf_rws * sizeof(PayloadBuffer));
        mem->hdl = handle;
        mem->stage = (uint8_t)stage;
        mem->slot = slot;

        ++active_counts.buf_rws;
        return *this;

    }
    Filler& Filler::add_sampler(ShaderStage stage, uint8_t slot, SamplerHandle handle)
    {
        assert(active_counts.samplers < hdr.counts.samplers);

        char* samplers_start = payload_start + samplers_offset;
        PayloadSampler* mem = (PayloadSampler*)(samplers_start + active_counts.samplers * sizeof(PayloadSampler));
        mem->hdl = handle;
        mem->stage = (uint8_t)stage;
        mem->slot = slot;

        ++active_counts.samplers;
        return *this;
    }

}






