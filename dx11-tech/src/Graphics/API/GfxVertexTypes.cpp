#include "pch.h"
#include "Graphics/API/GfxVertexTypes.h"

std::vector<D3D11_INPUT_ELEMENT_DESC> Vertex_POS_UV_NORMAL::get_desc(UINT buffer_slot, D3D11_INPUT_CLASSIFICATION input_slot_class)
{
	return
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, buffer_slot, 0, input_slot_class, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, buffer_slot, D3D11_APPEND_ALIGNED_ELEMENT, input_slot_class, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, buffer_slot, D3D11_APPEND_ALIGNED_ELEMENT, input_slot_class, 0 },
		//{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, buffer_slot, D3D11_APPEND_ALIGNED_ELEMENT, input_slot_class, 0 },
		//{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, buffer_slot, D3D11_APPEND_ALIGNED_ELEMENT, input_slot_class, 0 }
	};
}