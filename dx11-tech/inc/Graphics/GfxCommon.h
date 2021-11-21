#pragma once
#include "Graphics/DXDevice.h"

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
struct PipelineHandle : public InternalID 
{
	PipelineHandle() = default;
	PipelineHandle(uint64_t new_id) : InternalID(new_id) {} 
};

enum class ShaderStage
{
	eInvalid,
	eVertex,
	eHull,
	eDomain,
	eGeometry,
	ePixel,
	eCompute
};

enum class BAccess
{
	eReadImmutable,		// VB/IB
	eConstant,			// Constant Buffer
	eRead,				// SRV
	eReadWrite			// UAV
};

enum class TAccess
{
	eRead,				// SRV
	eReadWrite			// UAV
};

enum class TextureType
{
	eInvalid,
	e1D,
	e2D,
	e3D
};

struct BufferDesc
{
	UINT size = 0;
	D3D11_USAGE usage;
	UINT bind_flags = 0;
	UINT cpu_access_flags = 0;
	UINT misc_flags = 0;
	UINT structure_byte_stride = 0;

	D3D11_SUBRESOURCE_DATA subres = { nullptr, 0, 0 };

	static BufferDesc make_constant(UINT size);
	static BufferDesc make_vertex(void* data, UINT size);
	static BufferDesc make_index(void* data, UINT size);
};

struct TextureDesc
{
	TextureType type = TextureType::eInvalid;

	struct Common
	{
		UINT width = 0;
		UINT height = 0;
		DXGI_FORMAT format;
		UINT usage = 0;
		UINT bind_flags = 0;
		UINT cpu_access_flags = 0;
		UINT misc_flags = 0;
	} common;

	D3D11_SUBRESOURCE_DATA subres = { nullptr, 0, 0 };

	union
	{
		struct u1D
		{
			UINT array_size = 1;
		} u1D;

		struct u2D
		{
			UINT array_size = 1;
			UINT mip_levels = 1;
			DXGI_SAMPLE_DESC sample_desc = { 1, 0 };
		} u2D;

		struct u3D
		{
			UINT depth = 1;
			UINT mip_levels = 1;
		} u3D;
	};

	static TextureDesc make_1d(
		void* data, UINT size,
		UINT width, UINT height, DXGI_FORMAT format, UINT usage, UINT bind_flags, 
		UINT array_size = 1, 
		UINT cpu_access_flags = 0, UINT misc_flags = 0);

	static TextureDesc make_2d(
		void* data, UINT size,
		UINT width, UINT height, DXGI_FORMAT format, UINT usage, UINT bind_flags,
		UINT array_size = 1, UINT mip_levels = 1, DXGI_SAMPLE_DESC sample_desc = { 1, 0 },
		UINT cpu_access_flags = 0, UINT misc_flags = 0);

	static TextureDesc make_3d(
		void* data, UINT size,
		UINT width, UINT height, DXGI_FORMAT format, UINT usage, UINT bind_flags,
		UINT depth = 1, UINT mip_levels = 1,
		UINT cpu_access_flags = 0, UINT misc_flags = 0);
private:
	void set_common(UINT width, UINT height, DXGI_FORMAT format, UINT usage, UINT bind_flags, UINT cpu_access_flags, UINT misc_flags);

};

struct PipelineDesc
{
	D3D11_PRIMITIVE_TOPOLOGY topology;

	std::vector<D3D11_INPUT_ELEMENT_DESC> input_layout_desc;
	ShaderHandle shader;

	D3D11_RASTERIZER_DESC1 rasterizer_desc;
	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
	D3D11_BLEND_DESC1 blend_desc;
	
	// we should add reflection so the input layout desc is most of the time optional
	// we have the freedom to add input layout desc if we want to incorporate instanced rendering
	static PipelineDesc make(
		ShaderHandle program,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout_desc = {},
		const D3D11_RASTERIZER_DESC1 & rasterizer_desc = CD3D11_RASTERIZER_DESC1(),
		const D3D11_DEPTH_STENCIL_DESC & depth_stencil_desc = CD3D11_DEPTH_STENCIL_DESC(),
		const D3D11_BLEND_DESC1 & blend_desc = CD3D11_BLEND_DESC1());
};
