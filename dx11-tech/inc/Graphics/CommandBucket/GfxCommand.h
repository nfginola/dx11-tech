#pragma once
#include "Graphics/API/GfxHandles.h"
#include "Graphics/API/GfxCommon.h"

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
			Look at aux::bindtable
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

	struct ComputeDispatch
	{
		static const GfxCommandDispatch DISPATCH;
		ComputePipelineHandle pipeline;
		uint32_t x_blocks = 1;
		uint32_t y_blocks = 1;
		uint32_t z_blocks = 1;
		std::array<char, 32> profile_name;

		void set_profile_name(const char* name)
		{
			strncpy_s(profile_name.data(), profile_name.size(), "compute_test", profile_name.size());
		}
	};


	// Auxiliary memory helpers
	namespace aux
	{
		namespace bindtable
		{
			struct PayloadVB
			{
				BufferHandle hdl;
				uint32_t stride;
				uint32_t offset;
			};

			struct PayloadCB
			{
				BufferHandle hdl;
				uint8_t stage;
				uint8_t slot;
				uint32_t offset56s;
				uint32_t range56s;
			};

			struct PayloadSampler
			{
				SamplerHandle hdl;
				uint8_t stage;
				uint8_t slot;
			};

			struct PayloadTexture
			{
				TextureHandle hdl;
				uint8_t stage;
				uint8_t slot;
			};

			struct PayloadBuffer
			{
				BufferHandle hdl;
				uint8_t stage;
				uint8_t slot;
			};

			struct ResourceCounts
			{
				unsigned int vbs : 4;		// max 16 vb inputs

				unsigned int cbs : 4;		// max (16 - 1) cbs

				// max 128 bound srvs at any time (note that (tex_reads + buf_reads) < 128)
				unsigned int tex_reads : 7;
				unsigned int buf_reads : 7;			// shave off 1 bit to stay <= 32 bit (4 bytes)

				unsigned int tex_rws : 6;
				unsigned int buf_rws : 6;

				// max 16 samplers
				unsigned int samplers : 4;

				ResourceCounts() : vbs(0), cbs(0), tex_reads(0), buf_reads(0), tex_rws(0), buf_rws(0), samplers(0) {};
				size_t size() const
				{
					return
						vbs * sizeof(PayloadVB) +
						cbs * sizeof(PayloadCB) +
						tex_reads * sizeof(PayloadTexture) +
						buf_reads * sizeof(PayloadBuffer) +
						tex_rws * sizeof(PayloadTexture) +
						buf_rws * sizeof(PayloadBuffer);
				}

				bool operator==(const ResourceCounts& other) const
				{
					return
						(vbs == other.vbs) &&
						(cbs == other.cbs) &&
						(tex_reads == other.tex_reads) &&
						(buf_reads == other.buf_reads) &&
						(tex_rws == other.tex_rws) &&
						(buf_rws == other.buf_rws) &&
						(samplers == other.samplers);
				}
			
			};

			struct Header
			{
				friend struct Filler;
			public:
				Header() = default;

				// builder
				Header& set_vbs(uint8_t vb_count) { counts.vbs = vb_count; return *this; };
				Header& set_cbs(uint8_t cb_count) { counts.cbs = cb_count; return *this; };
				Header& set_tex_reads(uint8_t tex_read_count) { counts.tex_reads = tex_read_count; return *this; };
				Header& set_buf_reads(uint8_t buf_read_count) { counts.buf_reads = buf_read_count; return *this; };
				Header& set_tex_rws(uint8_t tex_rw_count) { counts.tex_rws = tex_rw_count; return *this; };
				Header& set_buf_rws(uint8_t buf_rw_count) { counts.buf_rws = buf_rw_count; return *this; };

				// Returns (header + payload) size
				size_t size() const { return sizeof(Header) + counts.size(); }

				const ResourceCounts& get_counts() const { return counts; }

			private:
				ResourceCounts counts;
			};

			struct Filler
			{
			public:
				Filler() = delete;
				~Filler();
				Filler(void* memory, const Header& header);

				// order determines binding slot [0, 15]
				Filler& add_vb(BufferHandle handle, uint32_t stride, uint32_t offset);

				Filler& add_cb(ShaderStage stage, uint8_t slot, BufferHandle handle, uint32_t offset56s = 0, uint32_t range56s = 1);
				Filler& add_read_tex(ShaderStage stage, uint8_t slot, TextureHandle handle);
				Filler& add_read_buf(ShaderStage stage, uint8_t slot, BufferHandle handle);
				Filler& add_rw_tex(ShaderStage stage, uint8_t slot, TextureHandle handle);
				Filler& add_rw_buf(ShaderStage stage, uint8_t slot, BufferHandle handle);
				Filler& add_sampler(ShaderStage stage, uint8_t slot, SamplerHandle handle);

	
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

					If none, order is still preserved
				*/
			private:
				const Header& hdr;
				char* const payload_start;

				// offset from payload start
				const uint16_t vb_offset = 0;
				const uint16_t cb_offset = (uint16_t)(vb_offset + hdr.counts.vbs * sizeof(PayloadVB));
				const uint16_t tex_read_offset = (uint16_t)(cb_offset + hdr.counts.cbs * sizeof(PayloadCB));
				const uint16_t buf_read_offset = (uint16_t)(tex_read_offset + hdr.counts.tex_reads * sizeof(PayloadTexture));
				const uint16_t tex_rws_offset = (uint16_t)(buf_read_offset + hdr.counts.buf_reads * sizeof(PayloadBuffer));
				const uint16_t buf_rws_offset = (uint16_t)(tex_rws_offset + hdr.counts.tex_rws * sizeof(PayloadTexture));
				const uint16_t samplers_offset = (uint16_t)(buf_rws_offset + hdr.counts.buf_rws * sizeof(PayloadBuffer));

				// currently bound resources: used to offset per bind into the payload
				ResourceCounts active_counts;

			};


		}


	}

}
