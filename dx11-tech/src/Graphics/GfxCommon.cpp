#include "pch.h"
#include "Graphics/GfxCommon.h"
#include "Graphics/dx.h"

BufferDesc BufferDesc::make_constant(UINT size)
{
	size = size + (16 - (size % 16));	// 16 bytes align

	BufferDesc b_d{};
	b_d.size = size;
	b_d.usage = D3D11_USAGE_DYNAMIC;
	b_d.bind_flags = D3D11_BIND_CONSTANT_BUFFER;
	b_d.cpu_access_flags = D3D11_CPU_ACCESS_WRITE;
	b_d.subres.pSysMem = nullptr;
	b_d.subres.SysMemPitch = 0;

	return BufferDesc();
}

BufferDesc BufferDesc::make_index(void* data, UINT size)
{
	// assuming uint32_t for indices
	BufferDesc b_d{};
	b_d.size = size;
	b_d.usage = D3D11_USAGE_IMMUTABLE;
	b_d.bind_flags = D3D11_BIND_INDEX_BUFFER;
	b_d.subres.pSysMem = data;
	b_d.subres.SysMemPitch = size;

	return b_d;
}

BufferDesc BufferDesc::make_vertex(void* data, UINT size)
{
	BufferDesc b_d{};
	b_d.size = size;
	b_d.usage = D3D11_USAGE_IMMUTABLE;
	b_d.bind_flags = D3D11_BIND_VERTEX_BUFFER;
	b_d.subres.pSysMem = data;
	b_d.subres.SysMemPitch = size;

	return b_d;
}


TextureDesc TextureDesc::make_1d(
	void* data, UINT size,
	UINT width, UINT height, DXGI_FORMAT format, UINT usage, UINT bind_flags,
	UINT array_size,
	UINT cpu_access_flags, UINT misc_flags)
{
	TextureDesc t_d{};
	t_d.set_common(width, height, format, usage, bind_flags, cpu_access_flags, misc_flags);
	t_d.type = TextureType::e1D;

	t_d.u1D.array_size = array_size;

	return t_d;
}

TextureDesc TextureDesc::make_2d(
	void* data, UINT size,
	UINT width, UINT height, DXGI_FORMAT format, UINT usage, UINT bind_flags,
	UINT array_size, UINT mip_levels, DXGI_SAMPLE_DESC sample_desc,
	UINT cpu_access_flags, UINT misc_flags)
{
	TextureDesc t_d{};
	t_d.set_common(width, height, format, usage, bind_flags, cpu_access_flags, misc_flags);
	t_d.type = TextureType::e2D;

	t_d.u2D.array_size = array_size;
	t_d.u2D.mip_levels = 1;
	t_d.u2D.sample_desc = sample_desc;

	return t_d;
}

TextureDesc TextureDesc::make_3d(
	void* data, UINT size,
	UINT width, UINT height, DXGI_FORMAT format, UINT usage, UINT bind_flags,
	UINT depth, UINT mip_levels,
	UINT cpu_access_flags, UINT misc_flags)
{
	TextureDesc t_d{};
	t_d.set_common(width, height, format, usage, bind_flags, cpu_access_flags, misc_flags);
	t_d.type = TextureType::e3D;

	t_d.u3D.depth = depth;
	t_d.u3D.mip_levels = mip_levels;

	return t_d;
}

void TextureDesc::set_common(UINT width, UINT height, DXGI_FORMAT format, UINT usage, UINT bind_flags, UINT cpu_access_flags, UINT misc_flags)
{
	common.width = width;
	common.height = height;
	common.format = format;
	common.usage = usage;
	common.bind_flags = bind_flags;
	common.cpu_access_flags = cpu_access_flags;
	common.misc_flags = misc_flags;
}

PipelineDesc PipelineDesc::make(
	ShaderHandle program,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout_desc,
	const D3D11_RASTERIZER_DESC1& rasterizer_desc,
	const D3D11_DEPTH_STENCIL_DESC& depth_stencil_desc,
	const D3D11_BLEND_DESC1& blend_desc)
{
	PipelineDesc d{};

	d.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	d.shader = program;
	d.input_layout_desc = input_layout_desc;
	
	d.rasterizer_desc = rasterizer_desc;
	d.depth_stencil_desc = depth_stencil_desc;
	d.blend_desc = blend_desc;
	return d;
}
