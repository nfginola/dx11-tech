#pragma once
#include "Graphics/API/GfxHandles.h"
#include <vector>
#include <tuple>

struct DrawItem
{
	// State
	PipelineHandle pipeline;

	// Geometry
	std::vector<std::tuple<BufferHandle, uint32_t, uint32_t>> vbs_strides_offsets;		// Should also be replaced with a void* for variable size array from linear alloc
	BufferHandle ib;
	uint32_t index_start = 0;
	uint32_t index_count = 0;
	uint32_t vertex_start = 0;

	// Resources (slot, resource)
	/*
		Should be replaced with three different memory allocs to some linear memory alloc for variable size array
		Below should be three void* 
	*/
	std::vector<std::tuple<uint8_t, ShaderStage, BufferHandle>> constant_buffers;
	std::vector<std::tuple<uint8_t, ShaderStage, SamplerHandle>> samplers;
	std::vector<std::tuple<uint8_t, ShaderStage, TextureHandle>> textures;

	/*
		Scissors
		Viewports
	*/
};

