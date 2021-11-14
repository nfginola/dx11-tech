#include "pch.h"
#include "Graphics/DXDevice.h"
#include "Graphics/dx.h"
#include "Graphics/DXShader.h"

dx* dx::s_self = nullptr;

dx::dx(unique_ptr<DXDevice> dev) :
	m_dev(std::move(dev))
{

}

void dx::init(unique_ptr<DXDevice> dev)
{
	s_self = new dx(std::move(dev));
}

void dx::shutdown()
{
	delete s_self;
	s_self = nullptr;
}

dx* dx::get()
{
	if (!s_self)
		assert(false);
	return s_self;
}

void dx::clear_backbuffer(DirectX::XMVECTORF32 color)
{
	m_dev->get_context()->ClearRenderTargetView(m_dev->get_bb_target().Get(), color);
}

void dx::present(bool vsync)
{
	m_dev->get_sc()->Present(vsync ? 1 : 0, 0);
}

BufferID dx::create_vertex_buffer()
{
	std::cout << "creating vb\n";
	return { (uint64_t)rand() };
}

BufferID dx::create_index_buffer()
{
	std::cout << "creating ib\n";
	return { (uint64_t)rand() };

}

BufferID dx::create_buffer()
{
	std::cout << "create generic buffer\n";
	return { (uint64_t)rand() };
}

TextureID dx::create_texture()
{
	std::cout << "create generic texture\n";
	return { (uint64_t)rand() };
}

ShaderID dx::create_shader(const std::filesystem::path& vs_path, const std::filesystem::path& ps_path, const std::filesystem::path& hs_path, const std::filesystem::path& ds_path, const std::filesystem::path& gs_path)
{
	DXShader shader(m_dev.get(), vs_path, ps_path, gs_path, hs_path, ds_path);

	return { (uint64_t)rand() };
}

void dx::upload_to_buffer(void* data, uint64_t size, BufferID id)
{
	std::cout << "uploading data from (0x" << std::hex << data << std::dec << ") with size " << size << " to buffer (" << id << ")" << std::endl;

	/*
	
		We should here then use some generic upload_resource() since Map/Unmap and UpdateSubresource both just take in an ID3D11Resource..
		
		After we find the correct Buffer/Texture from the BufferID or TextureID respectively..
		>>
		Some private function:

		upload_resource(ID3D11Resource* res, ...);
		*/
}

void dx::bind_buffer(uint8_t slot, ShaderStage stage, BufferID id)
{

	switch (stage)
	{
	case ShaderStage::Vertex:
		std::cout << "bound buffer (" << id << ") at slot (" << std::to_string(slot) << ") for Vertex Shader" << std::endl;
		break;
	case ShaderStage::Pixel:
		std::cout << "bound buffer (" << id << ") at slot (" << std::to_string(slot) << ") for Pixel Shader" << std::endl;
		break;
	case ShaderStage::Geometry:
		std::cout << "bound buffer (" << id << ") at slot (" << std::to_string(slot) << ") for Geometry Shader" << std::endl;
		break;
	case ShaderStage::Hull:
		std::cout << "bound buffer (" << id << ") at slot (" << std::to_string(slot) << ") for Hull Shader" << std::endl;
		break;
	case ShaderStage::Domain:
		std::cout << "bound buffer (" << id << ") at slot (" << std::to_string(slot) << ") for Domain Shader" << std::endl;
		break;
	case ShaderStage::Compute:
		std::cout << "bound buffer (" << id << ") at slot (" << std::to_string(slot) << ") for Compute Shader" << std::endl;
		break;
	}

}

void dx::bind_vertex_buffer(BufferID id)
{
	std::cout << "bound vertex buffer " << id << std::endl;

}

void dx::bind_index_buffer(BufferID id)
{
	std::cout << "bound index buffer " << id << std::endl;

}

void dx::bind_texture(uint8_t slot, ShaderStage stage, TextureID id)
{
	switch (stage)
	{
	case ShaderStage::Vertex:
		std::cout << "bound texture (" << id << ") at slot (" << std::to_string(slot) << ") for Vertex Shader" << std::endl;
		break;
	case ShaderStage::Pixel:
		std::cout << "bound texture (" << id << ") at slot (" << std::to_string(slot) << ") for Pixel Shader" << std::endl;
		break;
	case ShaderStage::Geometry:
		std::cout << "bound texture (" << id << ") at slot (" << std::to_string(slot) << ") for Geometry Shader" << std::endl;
		break;
	case ShaderStage::Hull:
		std::cout << "bound texture (" << id << ") at slot (" << std::to_string(slot) << ") for Hull Shader" << std::endl;
		break;
	case ShaderStage::Domain:
		std::cout << "bound texture (" << id << ") at slot (" << std::to_string(slot) << ") for Domain Shader" << std::endl;
		break;
	case ShaderStage::Compute:
		std::cout << "bound texture (" << id << ") at slot (" << std::to_string(slot) << ") for Compute Shader" << std::endl;
		break;
	}
}
