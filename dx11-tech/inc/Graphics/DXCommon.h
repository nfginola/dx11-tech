#pragma once

struct InternalID
{
	uint64_t id;
	operator uint64_t() { return id; }
};

/*
	Strongly typed IDs for safety
*/
struct BufferID : public InternalID {};
struct TextureID : public InternalID {};
struct ShaderID : public InternalID {};
struct PipelineStateID : public InternalID {};

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