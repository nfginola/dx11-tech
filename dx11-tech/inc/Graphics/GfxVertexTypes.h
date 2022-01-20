#pragma once
#include "Graphics/GfxCommon.h"


// Vertex types
struct Vertex_POS_UV_NORMAL
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
	//DirectX::XMFLOAT3 tangent;
	//DirectX::XMFLOAT3 bitangent;

	static std::vector<D3D11_INPUT_ELEMENT_DESC> get_desc(UINT buffer_slot, D3D11_INPUT_CLASSIFICATION input_slot_class);
};

