#include "pch.h"
#include "Graphics/API/GfxDescriptorsPrimitive.h"

BufferDesc BufferDesc::constant(size_t size_in_bytes, bool dynamic)
{
	size_in_bytes = size_in_bytes + (16 - (size_in_bytes % 16));	// 16 bytes align
	if (dynamic)
		return BufferDesc(CD3D11_BUFFER_DESC((UINT)size_in_bytes, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), BufferType::eConstant);
	else
		return BufferDesc(CD3D11_BUFFER_DESC((UINT)size_in_bytes, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DEFAULT), BufferType::eConstant);

}

BufferDesc BufferDesc::index(size_t size_in_bytes, bool dynamic)
{
	if (dynamic)
		return BufferDesc(CD3D11_BUFFER_DESC((UINT)size_in_bytes, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), BufferType::eIndex);
	else
		return BufferDesc(CD3D11_BUFFER_DESC((UINT)size_in_bytes, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_IMMUTABLE), BufferType::eIndex);
}

BufferDesc BufferDesc::vertex(size_t size_in_bytes, bool dynamic)
{
	if (dynamic)
		return BufferDesc(CD3D11_BUFFER_DESC((UINT)size_in_bytes, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), BufferType::eVertex);
	else
		return BufferDesc(CD3D11_BUFFER_DESC((UINT)size_in_bytes, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE), BufferType::eVertex);
}

BufferDesc BufferDesc::structured(size_t size_per_element, const std::pair<UINT, UINT> start_and_count, UINT bind_flags, bool cpu_dynamic)
{
	// 4 byte align (must be multiple of 4 if MiscFlag Structured specified)
	if (size_per_element % 4 != 0)
		size_per_element = size_per_element + (4 - (size_per_element % 4));

	assert(size_per_element <= 2048);
	UINT total_bytes = (UINT)size_per_element * (start_and_count.second - start_and_count.first);	// total elements

	bool gpu_dynamic = ((bind_flags & D3D11_BIND_UNORDERED_ACCESS) == D3D11_BIND_UNORDERED_ACCESS) ? true : false;

	if (cpu_dynamic)
	{
		auto desc = BufferDesc(CD3D11_BUFFER_DESC((UINT)total_bytes, bind_flags, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE,
			D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, (UINT)size_per_element),
			BufferType::eStructured);
		desc.m_start_and_count = start_and_count;
		return desc;
	}
	else
	{
		if (gpu_dynamic)
		{
			auto desc = BufferDesc(CD3D11_BUFFER_DESC((UINT)total_bytes, bind_flags, D3D11_USAGE_DEFAULT,
				0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, (UINT)size_per_element),
				BufferType::eStructured);
			desc.m_start_and_count = start_and_count;
			return desc;
		}
		else
		{
			auto desc = BufferDesc(CD3D11_BUFFER_DESC((UINT)total_bytes, bind_flags, D3D11_USAGE_IMMUTABLE,
				0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, (UINT)size_per_element),
				BufferType::eStructured);
			desc.m_start_and_count = start_and_count;
			return desc;
		}
	}
}

BufferDesc BufferDesc::staging(size_t size_in_bytes)
{
	return BufferDesc(CD3D11_BUFFER_DESC((UINT)size_in_bytes, 0, D3D11_USAGE_STAGING,
		D3D11_CPU_ACCESS_READ, 0, 0),
		BufferType::eStaging);
}

TextureDesc TextureDesc::depth_stencil(DepthFormat format, UINT width, UINT height, UINT bind_flags, UINT mip_levels, UINT sample_count, UINT sample_quality)
{
	// Depth stencil
	/*
		https://www.gamedev.net/forums/topic/691579-24bit-depthbuffer-is-a-sub-optimal-format/
		https://developer.nvidia.com/content/depth-precision-visualized

		32-bit depth + 8 bit stencil should be default.
		In create texture:
			- SRV should interpret the texture as DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS (32 bit depth readable)
			- DSV should interpret the texture as DXGI_FORMAT_D32_FLOAT_S8X24_UINT (32 bit depth + 8 bit stencil)
	*/

	if (format == DepthFormat::eD32)
		return TextureDesc(CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32_TYPELESS, width, height, 1, mip_levels, bind_flags, 
			D3D11_USAGE_DEFAULT, 0, sample_count, sample_quality, 0));
	else if (format == DepthFormat::eD32_S8)
		return TextureDesc(CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32G8X24_TYPELESS, width, height, 1, mip_levels, bind_flags, 
			D3D11_USAGE_DEFAULT, 0, sample_count, sample_quality, 0));
	else if (format == DepthFormat::eD24_S8)
		return TextureDesc(CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R24G8_TYPELESS, width, height, 1, mip_levels, bind_flags,
			D3D11_USAGE_DEFAULT, 0, sample_count, sample_quality, 0));
	else
	{
		assert(false);
		return CD3D11_TEXTURE2D_DESC{};
	}
}

TextureDesc TextureDesc::make_2d(DXGI_FORMAT format, UINT width, UINT height, UINT bind_flags, UINT mip_levels, UINT array_size, D3D11_USAGE usage, UINT cpu_access_flags, UINT sample_count, UINT sample_quality, UINT misc_flags)
{
	return TextureDesc(CD3D11_TEXTURE2D_DESC(format, width, height, array_size, mip_levels, bind_flags, usage, cpu_access_flags, sample_count, sample_quality, misc_flags));
}

InputLayoutDesc& InputLayoutDesc::append(LPCSTR semantic, DXGI_FORMAT format, UINT slot, InputClass input_type, UINT instanced_steprate)
{
	D3D11_INPUT_ELEMENT_DESC desc{};
	desc.SemanticName = semantic;
	desc.SemanticIndex = 0;
	desc.Format = format;
	desc.InputSlot = slot;
	desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	desc.InputSlotClass = input_type == InputClass::ePerVertex ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;		
	desc.InstanceDataStepRate = 0;
	m_input_descs.push_back(desc);

	return *this;
}

InputLayoutDesc& InputLayoutDesc::append(const InputLayoutDesc& another_layout)
{
	for (const auto& other_element_layout : another_layout.m_input_descs)
	{
		m_input_descs.push_back(other_element_layout);
	}
	return *this;
}
