#pragma once
#include "Graphics/DXDevice.h"
#include <variant>

//struct InternalID
//{
//protected:
//	uint64_t id;
//	InternalID(uint64_t new_id) : id(new_id) {};
//	InternalID() : id(0) {};
//public:
//	operator uint64_t() { return id; }
//};
//
///*
//	Strongly typed IDs for safety
//*/
//struct BufferHandle : public InternalID 
//{
//	BufferHandle() = default;
//	BufferHandle(uint64_t new_id) : InternalID(new_id) {}
//};
//
//struct TextureHandle : public InternalID 
//{
//	TextureHandle() = default;
//	TextureHandle(uint64_t new_id) : InternalID(new_id) {} 
//};
//
//struct ShaderHandle : public InternalID 
//{ 
//	ShaderHandle() = default;
//	ShaderHandle(uint64_t new_id) : InternalID(new_id) {} 
//};
//
//struct PipelineHandle : public InternalID 
//{
//	PipelineHandle() = default;
//	PipelineHandle(uint64_t new_id) : InternalID(new_id) {} 
//};
//


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

//enum class GPUAccess
//{
//	eRead,				// SRV
//	eReadWrite,			// UAV
//	eWrite				// RTV
//};

struct BufferDesc
{
	static BufferDesc make_constant(UINT size, void* data = nullptr);
	static BufferDesc make_vertex(UINT size, void* data = nullptr);
	static BufferDesc make_index(UINT size, void* data = nullptr);

	D3D11_BUFFER_DESC desc{};
	D3D11_SUBRESOURCE_DATA subres{ nullptr, 0, 0 };
};

struct SubresInit
{
	D3D11_SUBRESOURCE_DATA subres{ nullptr, 0, 0 };
	SubresInit(void* data = nullptr, UINT data_size = 0, UINT depth_size = 0) : subres({ data, data_size, depth_size }) { };
};

enum class TextureType { eInvalid, e1D, e2D, e3D };

struct TextureDesc
{
	static TextureDesc make_1d(const D3D11_TEXTURE1D_DESC& desc, const SubresInit& init_data = {});
	static TextureDesc make_2d(const D3D11_TEXTURE2D_DESC& desc, const SubresInit& init_data = {});
	static TextureDesc make_3d(const D3D11_TEXTURE3D_DESC& desc, const SubresInit& init_data = {});

	TextureType type = TextureType::eInvalid;
	std::variant<D3D11_TEXTURE1D_DESC, D3D11_TEXTURE2D_DESC, D3D11_TEXTURE3D_DESC> desc{};
	D3D11_SUBRESOURCE_DATA subres{ nullptr, 0, 0 };
};

//struct PipelineDesc
//{
//	D3D11_PRIMITIVE_TOPOLOGY topology;
//
//	std::vector<D3D11_INPUT_ELEMENT_DESC> input_layout_desc;
//	ShaderHandle shader;
//
//	D3D11_RASTERIZER_DESC1 rasterizer_desc;
//	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
//	D3D11_BLEND_DESC1 blend_desc;
//	
//	// we should add reflection so the input layout desc is most of the time optional
//	// we have the freedom to add input layout desc if we want to incorporate instanced rendering
//	static PipelineDesc make(
//		ShaderHandle program,
//		const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout_desc = {},
//		const D3D11_RASTERIZER_DESC1 & rasterizer_desc = CD3D11_RASTERIZER_DESC1(),
//		const D3D11_DEPTH_STENCIL_DESC & depth_stencil_desc = CD3D11_DEPTH_STENCIL_DESC(),
//		const D3D11_BLEND_DESC1 & blend_desc = CD3D11_BLEND_DESC1());
//};
