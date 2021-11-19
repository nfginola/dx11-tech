#include "pch.h"
#include "Graphics/dx.h"
#include "Graphics/DXDevice.h"
#include "Graphics/DXShader.h"
#include "Graphics/DXPipelineState.h"

dx* dx::s_self = nullptr;

namespace dx_null_views
{
	ID3D11ShaderResourceView* srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT] = { NULL };
	ID3D11RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };

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

void dx::begin_work()
{
	if (m_work_allowed)
		assert(false);		// please close the previous begin scope
	m_work_allowed = true;


}

void dx::end_work()
{
	if (!m_work_allowed)
		assert(false);		// please begin a scope
	m_work_allowed = false;
	
	// just unbind rtvs and dsv
	if (m_bound_RW.empty())
		unbind_writes_no_uav();

	// unbind uavs (and rtvs and dsv: D3D11 only allows us to do it this way)
	// we do single unbinds because we will allow flexible non contiguous slots for binding
	while (!m_bound_RW.empty())
	{
		auto data = m_bound_RW.front();
		unbind_writes_with_uav(data.first, data.second);
		m_bound_RW.pop();
	}
}

void dx::clear_backbuffer(DirectX::XMVECTORF32 color)
{
	validate_scope();

	//m_dev->get_context()->ClearView(m_dev->get_bb_target().Get(), color, NULL, 0);
	m_dev->get_context()->ClearRenderTargetView(m_dev->get_bb_target().Get(), color);
}

void dx::present(bool vsync)
{
	validate_scope();

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

PipelineStateHandle dx::create_pipeline()
{

	DXPipelineState pipeline;

	return PipelineStateHandle(rand());
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
	validate_scope();

	std::cout << "uploading data from (0x" << std::hex << data << std::dec << ") with size " << size << " to buffer (" << handle << ")" << std::endl;

	/*
	
		We should here then use some generic upload_resource() since Map/Unmap and UpdateSubresource both just take in an ID3D11Resource..
		
		After we find the correct Buffer/Texture from the BufferHandle or TextureHandle respectively..
		>>
		Some private function:

		upload_resource(ID3D11Resource* res, ...);
		*/
}

void dx::bind_buffer(uint8_t slot, ShaderStage stage, BufferHandle handle)
{
	validate_scope();

	switch (stage)
	{
	case ShaderStage::Vertex:
		std::cout << "bound buffer (" << handle << ") at slot (" << std::to_string(slot) << ") for Vertex Shader" << std::endl;
		break;
	case ShaderStage::Pixel:
		std::cout << "bound buffer (" << handle << ") at slot (" << std::to_string(slot) << ") for Pixel Shader" << std::endl;
		break;
	case ShaderStage::Geometry:
		std::cout << "bound buffer (" << handle << ") at slot (" << std::to_string(slot) << ") for Geometry Shader" << std::endl;
		break;
	case ShaderStage::Hull:
		std::cout << "bound buffer (" << handle << ") at slot (" << std::to_string(slot) << ") for Hull Shader" << std::endl;
		break;
	case ShaderStage::Domain:
		std::cout << "bound buffer (" << handle << ") at slot (" << std::to_string(slot) << ") for Domain Shader" << std::endl;
		break;
	case ShaderStage::Compute:
		std::cout << "bound buffer (" << handle << ") at slot (" << std::to_string(slot) << ") for Compute Shader" << std::endl;
		break;
	}

}

void dx::bind_vertex_buffer(BufferHandle handle)
{
	validate_scope();

	std::cout << "bound vertex buffer " << handle << std::endl;

}

void dx::bind_index_buffer(BufferHandle handle)
{
	validate_scope();
	std::cout << "bound index buffer " << handle << std::endl;

}

void dx::bind_texture(uint8_t slot, ShaderStage stage, TextureHandle handle)
{
	validate_scope();
	switch (stage)
	{
	case ShaderStage::Vertex:
		std::cout << "bound texture (" << handle << ") at slot (" << std::to_string(slot) << ") for Vertex Shader" << std::endl;
		break;
	case ShaderStage::Pixel:
		std::cout << "bound texture (" << handle << ") at slot (" << std::to_string(slot) << ") for Pixel Shader" << std::endl;
		break;
	case ShaderStage::Geometry:
		std::cout << "bound texture (" << handle << ") at slot (" << std::to_string(slot) << ") for Geometry Shader" << std::endl;
		break;
	case ShaderStage::Hull:
		std::cout << "bound texture (" << handle << ") at slot (" << std::to_string(slot) << ") for Hull Shader" << std::endl;
		break;
	case ShaderStage::Domain:
		std::cout << "bound texture (" << handle << ") at slot (" << std::to_string(slot) << ") for Domain Shader" << std::endl;
		break;
	case ShaderStage::Compute:
		std::cout << "bound texture (" << handle << ") at slot (" << std::to_string(slot) << ") for Compute Shader" << std::endl;
		break;
	}
}

void dx::draw_fullscreen_quad()
{
	validate_scope();
	std::cout << "draws fullscreen screenspace quad with an internally created VB\n";
}

void dx::bind_pipeline(PipelineStateHandle handle)
{
	validate_scope();
	std::cout << "binding pipeline (" << handle << ")\n";
}

void dx::bind_shader(ShaderHandle handle)
{
	validate_scope();
	std::cout << "binding shader (" << handle << ")\n";
}



void dx::create_default_resources()
{
	std::cout << "make sure to create default resources\n";

	// m_def_pipeline = ...

}

void dx::validate_scope()
{
	if (!m_work_allowed)
	{
		std::cout << "\n\n(GFX ERROR): API Command is not done within a scope!\n\n";
		assert(false);
	}
}

void dx::unbind_writes_with_uav(ShaderStage stage, uint32_t slot)
{
	ID3D11UnorderedAccessView* null_uav[] = { NULL };
	UINT initial_count = 0;

	switch (stage)
	{
	case ShaderStage::Vertex:
		m_dev->get_context()->OMSetRenderTargetsAndUnorderedAccessViews(
			_countof(dx_null_views::rtvs),
			dx_null_views::rtvs,
			nullptr,
			slot,
			1,
			null_uav,
			&initial_count
			);
		break;
	case ShaderStage::Pixel:
		m_dev->get_context()->OMSetRenderTargetsAndUnorderedAccessViews(
			_countof(dx_null_views::rtvs),
			dx_null_views::rtvs,
			nullptr,
			slot,
			1,
			null_uav,
			&initial_count
		);
		break;
	case ShaderStage::Geometry:
		m_dev->get_context()->OMSetRenderTargetsAndUnorderedAccessViews(
			_countof(dx_null_views::rtvs),
			dx_null_views::rtvs,
			nullptr,
			slot,
			1,
			null_uav,
			&initial_count
		);
		break;
	case ShaderStage::Hull:
		m_dev->get_context()->OMSetRenderTargetsAndUnorderedAccessViews(
			_countof(dx_null_views::rtvs),
			dx_null_views::rtvs,
			nullptr,
			slot,
			1,
			null_uav,
			&initial_count
		);
		break;
	case ShaderStage::Domain:
		m_dev->get_context()->OMSetRenderTargetsAndUnorderedAccessViews(
			_countof(dx_null_views::rtvs),
			dx_null_views::rtvs,
			nullptr,
			slot,
			1,
			null_uav,
			&initial_count
		);
		break;
	case ShaderStage::Compute:
		m_dev->get_context()->OMSetRenderTargets(
			_countof(dx_null_views::rtvs),
			dx_null_views::rtvs,
			nullptr
		);
		m_dev->get_context()->CSSetUnorderedAccessViews(slot, 1, null_uav, &initial_count);
		break;
	}

}

void dx::unbind_writes_no_uav()
{
	ID3D11RenderTargetView* null_rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };
	m_dev->get_context()->OMSetRenderTargets(
		D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
		null_rtvs,
		nullptr
	);
}
