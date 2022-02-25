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


}
