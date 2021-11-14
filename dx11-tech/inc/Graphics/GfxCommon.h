#pragma once

struct InternalID
{
	uint64_t id;
	operator uint64_t() { return id; }
};

/*
	Strongly typed IDs for safety
*/
struct BufferHandle : public InternalID {};
struct TextureHandle : public InternalID {};
struct ShaderHandle : public InternalID {};
struct PipelineStateHandle : public InternalID {};

enum class ShaderStage
{
	Invalid,
	Vertex,
	Hull,
	Domain,
	Geometry,
	Pixel,
	Compute
};