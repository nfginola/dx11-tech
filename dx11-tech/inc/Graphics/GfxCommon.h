#pragma once

struct InternalID
{
protected:
	uint64_t id;
	InternalID(uint64_t new_id) : id(new_id) {};
	InternalID() : id(0) {};
public:
	operator uint64_t() { return id; }
};

/*
	Strongly typed IDs for safety
*/
struct BufferHandle : public InternalID 
{
	BufferHandle() = default;
	BufferHandle(uint64_t new_id) : InternalID(new_id) {}
};
struct TextureHandle : public InternalID 
{
	TextureHandle() = default;
	TextureHandle(uint64_t new_id) : InternalID(new_id) {} 
};
struct ShaderHandle : public InternalID 
{ 
	ShaderHandle() = default;
	ShaderHandle(uint64_t new_id) : InternalID(new_id) {} 
};
struct PipelineStateHandle : public InternalID 
{
	PipelineStateHandle() = default;
	PipelineStateHandle(uint64_t new_id) : InternalID(new_id) {} 
};

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