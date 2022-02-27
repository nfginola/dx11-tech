#pragma once
#include "Graphics/API/GfxHandles.h"

typedef void (*GfxCommandDispatch)(const void*);

namespace gfxcommand
{
	struct Draw
	{
		static const GfxCommandDispatch DISPATCH;

		// State
		PipelineHandle pipeline;

		// Geometry
		BufferHandle ib;
		uint32_t index_start = 0;
		uint32_t index_count = 0;
		uint32_t vertex_start = 0;

		// Other resources which are dynamic uses the auxiliary memory (payload + header)
		/*
			VBs, CBs, Samplers, Read Textures, ...
		*/

	};

	struct CopyToBuffer
	{
		static const GfxCommandDispatch DISPATCH;

		// Will use map/unmap and can be extended to support update subresource later (through e.g simple flag)
		void* data = nullptr;
		size_t data_size = 0;
		BufferHandle buffer;
	};


	// Auxiliary memory helpers
	namespace aux
	{
		namespace bindtable
		{
			size_t get_size(uint8_t vb_count, uint8_t cb_count, uint8_t sampler_count, uint8_t texture_count);

			struct Header
			{
				unsigned int vbs : 4;
				unsigned int cbs : 4;
				unsigned int textures : 7;
				unsigned int validated : 1;
				unsigned int samplers : 3;

				Header() = delete;
				Header(uint8_t vbs_in, uint8_t cbs_in, uint8_t samplers_in, uint8_t textures_in)
				{
					validated = 0;
					vbs = vbs_in;
					cbs = cbs_in;
					samplers = samplers_in;
					textures = textures_in;
				}
			};

			struct Filler
			{
				Filler(void* start, uint8_t vbs_in, uint8_t cbs_in, uint8_t samplers_in, uint8_t textures_in);
				Filler& add_vb(res_handle handle, uint32_t stride, uint32_t offset);
				Filler& add_cb(res_handle handle, uint8_t stage, uint8_t slot, uint32_t offset56s = 0, uint32_t range56s = 1);
				Filler& add_texture(res_handle handle, uint8_t stage, uint8_t slot);
				Filler& add_sampler(res_handle handle, uint8_t stage, uint8_t slot);
				void validate();

				Header* hdr;
				char* payload_start = nullptr;
				size_t curr_payload_offset = 0;

				int vb_count = 0;
				int cb_count = 0;
				int sampler_count = 0;
				int texture_count = 0;

				bool vb_off = false;
				bool cb_off = false;
				bool sampler_off = false;
				bool textures_off = false;
			};

			struct PayloadVB
			{
				res_handle hdl;
				uint32_t stride;
				uint32_t offset;
			};

			struct PayloadCB
			{
				res_handle hdl;
				uint8_t stage;
				uint8_t slot;
				uint32_t offset56s;
				uint32_t range56s;
			};

			struct PayloadSampler
			{
				res_handle hdl;
				uint8_t stage;
				uint8_t slot;
			};

			struct PayloadTexture
			{
				res_handle hdl;
				uint8_t stage;
				uint8_t slot;
			};
		}




	}

}
