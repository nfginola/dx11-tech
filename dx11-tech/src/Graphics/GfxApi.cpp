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

void GfxApi::reload_shader(ShaderHandle hdl)
{
	m_shaders.find(hdl)->second->recompile(m_dev.get());
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
