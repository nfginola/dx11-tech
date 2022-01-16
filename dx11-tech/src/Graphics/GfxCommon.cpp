#include "pch.h"
#include "Graphics/GfxCommon.h"

BufferDesc BufferDesc::make_constant(UINT size, void* data)
{
	size = size + (16 - (size % 16));	// 16 bytes align
	BufferDesc b_d{};
	b_d.desc = CD3D11_BUFFER_DESC(size, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	b_d.subres.pSysMem = data;
	b_d.subres.SysMemPitch = size;
	b_d.subres.SysMemSlicePitch = 0;
	b_d.type = BufferType::eConstant;
	return b_d;
}

BufferDesc BufferDesc::make_index(UINT size, void* data)
{
	BufferDesc b_d{};
	b_d.desc = CD3D11_BUFFER_DESC(size, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_IMMUTABLE);
	b_d.subres.pSysMem = data;
	b_d.subres.SysMemPitch = size;
	b_d.subres.SysMemSlicePitch = 0;
	b_d.type = BufferType::eIndex;
	return b_d;
}

BufferDesc BufferDesc::make_vertex(UINT size, void* data)
{
	BufferDesc b_d{};
	b_d.desc = CD3D11_BUFFER_DESC(size, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE);
	b_d.subres.pSysMem = data;
	b_d.subres.SysMemPitch = size;
	b_d.subres.SysMemSlicePitch = 0;
	b_d.type = BufferType::eVertex;
	return b_d;
}

//
//PipelineDesc PipelineDesc::make(
//	ShaderHandle program,
//	const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout_desc,
//	const D3D11_RASTERIZER_DESC1& rasterizer_desc,
//	const D3D11_DEPTH_STENCIL_DESC& depth_stencil_desc,
//	const D3D11_BLEND_DESC1& blend_desc)
//{
//	PipelineDesc d{};
//
//	d.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//	d.shader = program;
//	d.input_layout_desc = input_layout_desc;
//	
//	d.rasterizer_desc = rasterizer_desc;
//	d.depth_stencil_desc = depth_stencil_desc;
//	d.blend_desc = blend_desc;
//	return d;
//}

TextureDesc TextureDesc::make_1d(const D3D11_TEXTURE1D_DESC& desc, const SubresInit& init_data)
{
	assert(false);
	return TextureDesc{};
}

TextureDesc TextureDesc::make_2d(const D3D11_TEXTURE2D_DESC& desc, const SubresInit& init_data)
{
	TextureDesc t_d{};
	t_d.desc = desc;
	t_d.subres = init_data.subres;
	t_d.type = TextureType::e2D;
	return t_d;
}

TextureDesc TextureDesc::make_3d(const D3D11_TEXTURE3D_DESC& desc, const SubresInit& init_data)
{
	assert(false);
	return TextureDesc{};
}

ViewDesc& ViewDesc::set(const D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
{
	srv_desc = desc;
	return *this;
}

ViewDesc& ViewDesc::set(const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc)
{
	uav_desc = desc;
	return *this;
}

ViewDesc& ViewDesc::set(const D3D11_RENDER_TARGET_VIEW_DESC& desc)
{
	rtv_desc = desc;
	return *this;
}

