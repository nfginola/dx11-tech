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

namespace gfxcommand::aux::bindtable
{
    size_t get_size(uint8_t vb_count, uint8_t cb_count, uint8_t sampler_count, uint8_t texture_count)
    {
        return sizeof(gfxcommand::aux::bindtable::Header) +
            sizeof(gfxcommand::aux::bindtable::PayloadVB) * vb_count +
            sizeof(gfxcommand::aux::bindtable::PayloadCB) * cb_count +
            sizeof(gfxcommand::aux::bindtable::PayloadSampler) * sampler_count +
            sizeof(gfxcommand::aux::bindtable::PayloadTexture) * texture_count;
    }

    gfxcommand::aux::bindtable::Filler::Filler(void* start, uint8_t vbs_in, uint8_t cbs_in, uint8_t samplers_in, uint8_t textures_in)
    {
        hdr = ((Header*)start);
        *hdr = Header(vbs_in, cbs_in, samplers_in, textures_in);

        payload_start = (char*)start + sizeof(Header);
    }

    gfxcommand::aux::bindtable::Filler& gfxcommand::aux::bindtable::Filler::add_vb(res_handle handle, uint32_t stride, uint32_t offset)
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

    gfxcommand::aux::bindtable::Filler& gfxcommand::aux::bindtable::Filler::add_cb(res_handle handle, uint8_t stage, uint8_t slot)
    {
        if (cb_off || cb_count >= hdr->cbs)
            assert(false);

        vb_off = true;
        void* curr_pos = payload_start + curr_payload_offset;

        ((PayloadCB*)curr_pos)->hdl = handle;
        ((PayloadCB*)curr_pos)->stage = stage;
        ((PayloadCB*)curr_pos)->slot = slot;

        curr_payload_offset += sizeof(PayloadCB);
        ++cb_count;
        return *this;

    }

    gfxcommand::aux::bindtable::Filler& gfxcommand::aux::bindtable::Filler::add_texture(res_handle handle, uint8_t stage, uint8_t slot)
    {
        if (textures_off || texture_count >= hdr->textures)
            assert(false);

        vb_off = true;
        cb_off = true;
        void* curr_pos = payload_start + curr_payload_offset;

        ((PayloadTexture*)curr_pos)->hdl = handle;
        ((PayloadTexture*)curr_pos)->stage = stage;
        ((PayloadTexture*)curr_pos)->slot = slot;

        curr_payload_offset += sizeof(PayloadTexture);
        ++texture_count;
        return *this;
    }

    gfxcommand::aux::bindtable::Filler& gfxcommand::aux::bindtable::Filler::add_sampler(res_handle handle, uint8_t stage, uint8_t slot)
    {
        if (sampler_count >= hdr->samplers)
            assert(false);

        vb_off = true;
        cb_off = true;
        textures_off = true;
        void* curr_pos = payload_start + curr_payload_offset;

        ((PayloadSampler*)curr_pos)->hdl = handle;
        ((PayloadSampler*)curr_pos)->stage = stage;
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



/*
    Other commands..
*/


