#include "pch.h"
#include "Graphics/dx.h"
#include "Graphics/DXDevice.h"
#include "Graphics/DXShader.h"
#include "Graphics/DXPipelineState.h"
#include "Graphics/DXTexture.h"

dx* dx::s_self = nullptr;

namespace dx_null_views
{
	ID3D11ShaderResourceView* srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT] = { NULL };
	ID3D11RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };
	ID3D11UnorderedAccessView* uavs[D3D11_PS_CS_UAV_REGISTER_COUNT] = { NULL };

}

dx::dx(unique_ptr<DXDevice> dev) :
	m_dev(std::move(dev))
{
	create_default_resources();
}

dx::~dx()
{
	/* clear all resources */
}






void dx::init(unique_ptr<DXDevice> dev)
{
	if (s_self)
		assert(false);
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

	//m_dev->get_context()->ClearView(m_dev->get_bb_target().Get(), color, NULL, 0);
	m_dev->get_context()->ClearRenderTargetView(m_dev->get_bb_target().Get(), color);
}

void dx::present(bool vsync)
{

	m_dev->get_sc()->Present(vsync ? 1 : 0, 0);
}

void dx::start_frame()
{
	// clean all previous frames bound read resources to avoid any RW bind conflicts
	m_dev->get_context()->VSSetShaderResources(0, _countof(dx_null_views::srvs), dx_null_views::srvs);
	m_dev->get_context()->HSSetShaderResources(0, _countof(dx_null_views::srvs), dx_null_views::srvs);
	m_dev->get_context()->DSSetShaderResources(0, _countof(dx_null_views::srvs), dx_null_views::srvs);
	m_dev->get_context()->GSSetShaderResources(0, _countof(dx_null_views::srvs), dx_null_views::srvs);
	m_dev->get_context()->PSSetShaderResources(0, _countof(dx_null_views::srvs), dx_null_views::srvs);
	m_dev->get_context()->CSSetShaderResources(0, _countof(dx_null_views::srvs), dx_null_views::srvs);
}

void dx::end_frame()
{

}

BufferHandle dx::create_vertex_buffer()
{
	BufferPtr b;

	D3D11_BUFFER_DESC b_d{};
	//b_d.ByteWidth = static_cast<UINT>(desc.total_size);
	//b_d.Usage = D3D11_USAGE_IMMUTABLE;
	//b_d.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vb_d{};
	//vb_d.pSysMem = desc.data;
	//vb_d.SysMemPitch = desc.ByteWidth;
	//HRCHECK(m_dev->get_device()->CreateBuffer(&b_d, &vb_d, b.GetAddressOf()));

	std::cout << "creating vb\n";
	return BufferHandle(rand());
}

BufferHandle dx::create_index_buffer()
{

	std::cout << "creating ib\n";
	return BufferHandle(rand());
}



ShaderHandle dx::create_shader(const std::filesystem::path& vs_path, const std::filesystem::path& ps_path, const std::filesystem::path& hs_path, const std::filesystem::path& ds_path, const std::filesystem::path& gs_path)
{

	DXShader shader(m_dev.get(), vs_path, ps_path, gs_path, hs_path, ds_path);

	return ShaderHandle(rand());
}

ShaderHandle dx::create_compute_shader(const std::filesystem::path& cs_path)
{
	assert(false);
	// to implement
	return ShaderHandle();
}

PipelineHandle dx::create_pipeline()
{

	DXPipelineState pipeline;

	return PipelineHandle(rand());
}

BufferHandle dx::create_buffer()
{

	std::cout << "create generic buffer\n";
	return BufferHandle(rand());
}

TextureHandle dx::create_texture()
{
	std::cout << "create generic texture\n";
	return TextureHandle(rand());
}

void dx::hot_reload_shader(ShaderHandle handle)
{
	std::cout << "hot reloading shader (" << handle << ")\n";
}

void dx::upload_to_buffer(void* data, uint64_t size, BufferHandle handle)
{
	std::cout << "uploading data from (0x" << std::hex << data << std::dec << ") with size " << size << " to buffer (" << handle << ")" << std::endl;

	/*
	
		We should here then use some generic upload_resource() since Map/Unmap and UpdateSubresource both just take in an ID3D11Resource..
		
		After we find the correct Buffer/Texture from the BufferHandle or TextureHandle respectively..
		>>
		Some private function:

		upload_resource(ID3D11Resource* res, ...);
		*/
}

void dx::bind_buffer(uint8_t slot, BAccess mode, ShaderStage stage, BufferHandle handle)
{

	//if (stage == ShaderStage::eCompute)
	//{
	//	if (mode == BAccess::eReadWrite)
	//		m_bound_RW.push({ stage, slot });
	//	else
	//	{
	//		std::cout << "(GFX ERROR): ReadWrite is not supported for any other stages than Compute stage\n";
	//		assert(false);
	//	}
	//}

	switch (stage)
	{
	case ShaderStage::eVertex:
		std::cout << "bound buffer (" << handle << ") at slot (" << std::to_string(slot) << ") for Vertex Shader" << std::endl;
		break;
	case ShaderStage::ePixel:
		std::cout << "bound buffer (" << handle << ") at slot (" << std::to_string(slot) << ") for Pixel Shader" << std::endl;
		break;
	case ShaderStage::eGeometry:
		std::cout << "bound buffer (" << handle << ") at slot (" << std::to_string(slot) << ") for Geometry Shader" << std::endl;
		break;
	case ShaderStage::eHull:
		std::cout << "bound buffer (" << handle << ") at slot (" << std::to_string(slot) << ") for Hull Shader" << std::endl;
		break;
	case ShaderStage::eDomain:
		std::cout << "bound buffer (" << handle << ") at slot (" << std::to_string(slot) << ") for Domain Shader" << std::endl;
		break;
	case ShaderStage::eCompute:


		std::cout << "bound buffer (" << handle << ") at slot (" << std::to_string(slot) << ") for Compute Shader" << std::endl;
		break;
	}

}

void dx::bind_vertex_buffer(BufferHandle handle)
{

	std::cout << "bound vertex buffer " << handle << std::endl;

}

void dx::bind_index_buffer(BufferHandle handle)
{
	std::cout << "bound index buffer " << handle << std::endl;

}

void dx::bind_texture(uint8_t slot, TAccess mode, ShaderStage stage, TextureHandle handle)
{

	if (mode == TAccess::eReadWrite)
	{
		//if (stage == ShaderStage::eCompute)
		//	m_bound_RW.push({ stage, slot });
		//else
		//{
		//	std::cout << "(GFX ERROR): ReadWrite is not supported for any other stages than Compute stage\n";
		//	assert(false);
		//}
	}

	switch (stage)
	{
	case ShaderStage::eVertex:
		std::cout << "bound texture (" << handle << ") at slot (" << std::to_string(slot) << ") for Vertex Shader" << std::endl;
		break;
	case ShaderStage::ePixel:
		std::cout << "bound texture (" << handle << ") at slot (" << std::to_string(slot) << ") for Pixel Shader" << std::endl;
		break;
	case ShaderStage::eGeometry:
		std::cout << "bound texture (" << handle << ") at slot (" << std::to_string(slot) << ") for Geometry Shader" << std::endl;
		break;
	case ShaderStage::eHull:
		std::cout << "bound texture (" << handle << ") at slot (" << std::to_string(slot) << ") for Hull Shader" << std::endl;
		break;
	case ShaderStage::eDomain:
		std::cout << "bound texture (" << handle << ") at slot (" << std::to_string(slot) << ") for Domain Shader" << std::endl;
		break;
	case ShaderStage::eCompute:
		std::cout << "bound texture (" << handle << ") at slot (" << std::to_string(slot) << ") for Compute Shader" << std::endl;
		break;
	}
}

void dx::draw_fullscreen_quad()
{
	std::cout << "draws fullscreen screenspace quad with an internally created VB\n";
}

void dx::bind_pipeline(PipelineHandle handle)
{
	std::cout << "binding pipeline (" << handle << ")\n";
}

void dx::bind_shader(ShaderHandle handle)
{
	std::cout << "binding shader (" << handle << ")\n";
}



void dx::create_default_resources()
{
	std::cout << "make sure to create default resources\n";

	// m_def_pipeline = ...

}


void dx::unbind_writes_with_uav(ShaderStage stage, uint32_t slot)
{
	UINT initial_count = 0;

	switch (stage)
	{
	case ShaderStage::eVertex:
		m_dev->get_context()->OMSetRenderTargetsAndUnorderedAccessViews(
			1,
			dx_null_views::rtvs,
			nullptr,
			slot,
			1,
			dx_null_views::uavs,
			&initial_count
			);
		break;
	case ShaderStage::ePixel:
		m_dev->get_context()->OMSetRenderTargetsAndUnorderedAccessViews(
			1,
			dx_null_views::rtvs,
			nullptr,
			0,
			1,
			dx_null_views::uavs,
			&initial_count
		);
		break;
	case ShaderStage::eGeometry:
		m_dev->get_context()->OMSetRenderTargetsAndUnorderedAccessViews(
			0,
			dx_null_views::rtvs,
			nullptr,
			slot,
			1,
			dx_null_views::uavs,
			&initial_count
		);
		break;
	case ShaderStage::eHull:
		m_dev->get_context()->OMSetRenderTargetsAndUnorderedAccessViews(
			0,
			dx_null_views::rtvs,
			nullptr,
			slot,
			1,
			dx_null_views::uavs,
			&initial_count
		);
		break;
	case ShaderStage::eDomain:
		m_dev->get_context()->OMSetRenderTargetsAndUnorderedAccessViews(
			0,
			dx_null_views::rtvs,
			nullptr,
			slot,
			1,
			dx_null_views::uavs,
			&initial_count
		);
		break;
	case ShaderStage::eCompute:
		m_dev->get_context()->OMSetRenderTargets(
			0,
			dx_null_views::rtvs,
			nullptr
		);
		m_dev->get_context()->CSSetUnorderedAccessViews(slot, 1, dx_null_views::uavs, &initial_count);
		break;
	}

}

void dx::unbind_rtvs_dsv()
{
	ID3D11RenderTargetView* null_rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };
	m_dev->get_context()->OMSetRenderTargets(
		D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
		null_rtvs,
		nullptr
	);
}
