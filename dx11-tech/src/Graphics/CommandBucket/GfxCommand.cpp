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
const GfxCommandDispatch gfxcommand::CopyToBuffer::DISPATCH = gfxcommand_dispatch::copy_to_buffer;

namespace gfxcommand::aux::bindtable
{
    size_t get_size(uint8_t vb_count, uint8_t cb_count, uint8_t sampler_count, uint8_t texture_count)
    {
        return sizeof(Header) +
            sizeof(PayloadVB) * vb_count +
            sizeof(PayloadCB) * cb_count +
            sizeof(PayloadSampler) * sampler_count +
            sizeof(PayloadTexture) * texture_count;
    }

    Filler::Filler(void* start, uint8_t vbs_in, uint8_t cbs_in, uint8_t samplers_in, uint8_t textures_in)
    {
        hdr = ((Header*)start);
        *hdr = Header(vbs_in, cbs_in, samplers_in, textures_in);

        payload_start = (char*)start + sizeof(Header);
    }

    Filler& Filler::add_vb(res_handle handle, uint32_t stride, uint32_t offset)
    {
        if (vb_off || vb_count >= hdr->vbs)
            assert(false);

        void* curr_pos = payload_start + curr_payload_offset;

        ((PayloadVB*)curr_pos)->hdl = handle;
        ((PayloadVB*)curr_pos)->stride = stride;
        ((PayloadVB*)curr_pos)->offset = offset;

        curr_payload_offset += sizeof(PayloadVB);
        ++vb_count;
        return *this;
    }

    Filler& Filler::add_cb(res_handle handle, ShaderStage stage, uint8_t slot, uint32_t offset56s, uint32_t range56s)
    {
        if (cb_off || cb_count >= hdr->cbs)
            assert(false);

        vb_off = true;
        void* curr_pos = payload_start + curr_payload_offset;

        ((PayloadCB*)curr_pos)->hdl = handle;
        ((PayloadCB*)curr_pos)->stage = (uint8_t)stage;
        ((PayloadCB*)curr_pos)->slot = slot;
        ((PayloadCB*)curr_pos)->offset56s = offset56s;
        ((PayloadCB*)curr_pos)->range56s = range56s;

        curr_payload_offset += sizeof(PayloadCB);
        ++cb_count;
        return *this;

    }

    Filler& Filler::add_texture(res_handle handle, ShaderStage stage, uint8_t slot)
    {
        if (textures_off || texture_count >= hdr->textures)
            assert(false);

        vb_off = true;
        cb_off = true;
        void* curr_pos = payload_start + curr_payload_offset;

        ((PayloadTexture*)curr_pos)->hdl = handle;
        ((PayloadTexture*)curr_pos)->stage = (uint8_t)stage;
        ((PayloadTexture*)curr_pos)->slot = slot;

        curr_payload_offset += sizeof(PayloadTexture);
        ++texture_count;
        return *this;
    }

    Filler& Filler::add_sampler(res_handle handle, ShaderStage stage, uint8_t slot)
    {
        if (sampler_count >= hdr->samplers)
            assert(false);

        vb_off = true;
        cb_off = true;
        textures_off = true;
        void* curr_pos = payload_start + curr_payload_offset;

        ((PayloadSampler*)curr_pos)->hdl = handle;
        ((PayloadSampler*)curr_pos)->stage = (uint8_t)stage;
        ((PayloadSampler*)curr_pos)->slot = slot;

        curr_payload_offset += sizeof(PayloadSampler);
        ++sampler_count;
        return *this;
    }

    void Filler::validate()
    {
        assert(vb_count == hdr->vbs);
        assert(cb_count == hdr->cbs);
        assert(sampler_count == hdr->samplers);
        assert(texture_count == hdr->textures);
        hdr->validated = 1;
    }

}

namespace gfxcommand::aux::computebindtable
{
    // validation
    ComputeFiller::~ComputeFiller()
    {
        // check so that we have added the bindings that we have declared
        assert(active_counts == hdr.counts);
    }

    ComputeFiller::ComputeFiller(void* memory, const ComputeHeader& header) :
        hdr(header),
        payload_start((char*)memory + sizeof(ComputeHeader))
    {
        // copy header to the start of the memory
        *(ComputeHeader*)memory = header;
    }

    ComputeFiller& ComputeFiller::add_vb(BufferHandle handle, uint32_t stride, uint32_t offset)
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
    ComputeFiller& ComputeFiller::add_cb(BufferHandle handle, ShaderStage stage, uint8_t slot, uint32_t offset56s, uint32_t range56s)
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
    ComputeFiller& ComputeFiller::add_read_tex(TextureHandle handle, ShaderStage stage, uint8_t slot)
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
    ComputeFiller& ComputeFiller::add_read_buf(BufferHandle handle, ShaderStage stage, uint8_t slot)
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
    ComputeFiller& ComputeFiller::add_rw_tex(TextureHandle handle, ShaderStage stage, uint8_t slot)
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
    ComputeFiller& ComputeFiller::add_rw_buf(BufferHandle handle, ShaderStage stage, uint8_t slot)
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
    ComputeFiller& ComputeFiller::add_sampler(SamplerHandle handle, ShaderStage stage, uint8_t slot)
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






