#include "pch.h"
#include "Graphics/GfxApi.h"

#include "Graphics/GfxCommon.h"
#include "Graphics/DXTexture.h"
#include "Graphics/DXShader.h"
#include "Graphics/DXBuffer.h"

uint64_t get_next_id(uint64_t& counter)
{
	auto id = counter;
	++counter;
	return id;
}

GfxApi::GfxApi(std::unique_ptr<DXDevice> dev) :
	m_dev(std::move(dev))
{
	m_buffers.reserve(500);
	m_textures.reserve(500);


	/*
	
	ezdx(make_unique dx device)
	...


	TEXTURE CREATION
	=======

	TextureHandle my_tex = ezdx->create_texture(TextureDesc::from(
	));
	ezdx->create_srv(my_tex, [](ID3D11Texture2D* tex) { return CD3D11(...); });

	TextureHandle tex_copy = ezdx->create_texture(my_tex);						// create from handle --> same underlying Texture2D but no views!
	ezdx->create_srv(tex_copy, [](ID3D11Texture2D* tex) { return CD3D11(...); }	// same tex2d, but different view!
	========

	TEXTURE BINDING
	========
	ezdx->bind_texture(Slot, Access, Stage, textureHandle);
		-- if bind read --> check if internal texture is already bound as write, if yes, unbind.
			-- if prev == readWrite --> unbind UAV for slot at stage
			-- if prev == write		--> unbind RTV fully
	========

	CONSTANT BUFFER (EXPLICIT)
	========

	ezdx->bind_constant_buffer(slot, stage, bufferHandle)
		-- internally checks the DXBuffer if it is of type Constant Buffer
	========

	OTHER BUFFER (VIEWS)
	=======
	ezdx->bind_buffer(slot, access, stage, bufferHandle)
		-- internally checks that access, bufferhandle and views all match and exist
	=======

	FRAMEBUFFERS (COLLECTION OF TEXTURE TO RENDER TO)
	=======
	FboHandle ezdx->create_fbo( [ { tex1, settings1 }, { tex2, settings2 } ] )
	
	ezdx->bind_fbo(fboHandle);

	ezdx->clear_fbo(fboHandle, { depth_stencil, [ tex1clear, tex2clear, ... ] });

	=======

	MISCELLANEOUS (FOR LATER) OPTIMIZATIONS
	=======
	ezdx->clear_constant_buffers()		// unbinds all bound cbuffers so that update latency is not slow! (it is dependent on how many places the res is bound)
	=======


	
	*/

	// Creating shader
	DXShader shader(m_dev.get(), "a.hlsl", "b.hlsl");
	
	// Creating texture and view for it
	DXTexture my_tex(m_dev.get(), TextureDesc::make_2d(CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_UNORM, 800, 600)));
	my_tex.create_srv_ext(m_dev.get(), [](ID3D11Texture2D* tex) { return CD3D11_SHADER_RESOURCE_VIEW_DESC(tex, D3D11_SRV_DIMENSION_TEXTURE2D); });
	
	// Test copy
	DXTexture ot(my_tex);

	// Test buffer
	DXBuffer cbuf(m_dev.get(), BufferDesc::make_constant(128));





	// Example mistakes, which are guarded against!
	// Same mistakes are guarded against for create_uav and create_rtv.
	//my_tex.create_srv_ext(m_dx_device.get(), [](ID3D11Texture2D* tex) { return CD3D11_SHADER_RESOURCE_VIEW_DESC(tex, D3D11_SRV_DIMENSION_TEXTURE1D); });
	//my_tex.create_srv_ext(m_dx_device.get(), [](ID3D11Texture1D* tex) { return CD3D11_SHADER_RESOURCE_VIEW_DESC(tex, D3D11_SRV_DIMENSION_TEXTURE2D); });

}

GfxApi::~GfxApi()
{
	// Drop all resources
	m_shaders.clear();
	m_buffers.clear();
	m_textures.clear();
}

ShaderHandle GfxApi::create_shader_program(const std::filesystem::path& vs_path, const std::filesystem::path& ps_path, const std::filesystem::path& gs_path, const std::filesystem::path& hs_path, const std::filesystem::path& ds_path)
{
	auto shader = make_unique<DXShader>(m_dev.get(), vs_path, ps_path, gs_path, hs_path, ds_path);

	auto id = get_next_id(m_next_shader_id);
	m_shaders.insert({ id, std::move(shader) });

	return id;
}

ShaderHandle GfxApi::create_compute_program(const std::filesystem::path& cs_path)
{
	auto shader = make_unique<DXShader>(m_dev.get(), cs_path);

	auto id = get_next_id(m_next_shader_id);
	m_shaders.insert({ id, std::move(shader) });

	return id;
}

BufferHandle GfxApi::create_buffer(const BufferDesc& desc, const ViewDesc& view)
{
	auto buf = make_unique<DXBuffer>(m_dev.get(), desc);

	if (view.srv_desc)
		buf->create_srv(m_dev.get(), view.srv_desc.value());
	if (view.uav_desc)
		buf->create_uav(m_dev.get(), view.uav_desc.value());

	auto id = get_next_id(m_next_buffer_id);
	m_buffers.insert({ id, std::move(buf) });

	return id;
}

BufferHandle GfxApi::create_buffer(BufferHandle hdl, const ViewDesc& view, bool use_underlying_resource)
{
	unique_ptr<DXBuffer> buf;
	if (use_underlying_resource)
		buf = make_unique<DXBuffer>(*(m_buffers.find(hdl)->second));
	else
		buf = make_unique<DXBuffer>(m_dev.get(), m_buffers.find(hdl)->second->get_desc());

	if (view.srv_desc)
		buf->create_srv(m_dev.get(), view.srv_desc.value());
	if (view.uav_desc)
		buf->create_uav(m_dev.get(), view.uav_desc.value());

	auto id = get_next_id(m_next_buffer_id);
	m_buffers.insert({ id, std::move(buf) });

	return id;
}

TextureHandle GfxApi::create_texture(const TextureDesc& desc, const ViewDesc& view)
{
	auto tex = make_unique<DXTexture>(m_dev.get(), desc);

	if (view.srv_desc)
		tex->create_srv(m_dev.get(), view.srv_desc.value());
	if (view.uav_desc)
		tex->create_uav(m_dev.get(), view.uav_desc.value());
	if (view.rtv_desc)
		tex->create_rtv(m_dev.get(), view.rtv_desc.value());

	auto id = get_next_id(m_next_texture_id);
	m_textures.insert({ id, std::move(tex) });

	return id;
}

TextureHandle GfxApi::create_texture(TextureHandle hdl, const ViewDesc& view, bool use_underlying_resource)
{
	unique_ptr<DXTexture> tex;
	if (use_underlying_resource)
		tex = make_unique<DXTexture>(*(m_textures.find(hdl)->second));
	else
		tex = make_unique<DXTexture>(m_dev.get(), m_textures.find(hdl)->second->get_desc());

	if (view.srv_desc)
		tex->create_srv(m_dev.get(), view.srv_desc.value());
	if (view.uav_desc)
		tex->create_uav(m_dev.get(), view.uav_desc.value());
	if (view.rtv_desc)
		tex->create_rtv(m_dev.get(), view.rtv_desc.value());

	auto id = get_next_id(m_next_texture_id);
	m_textures.insert({ id, std::move(tex) });

	return id;
}

void GfxApi::drop_shader_program(ShaderHandle hdl)
{
	m_shaders.erase(hdl);
}

void GfxApi::drop_buffer(BufferHandle hdl)
{
	m_buffers.erase(hdl);
}

void GfxApi::drop_texture(TextureHandle hdl)
{
	m_textures.erase(hdl);
}
