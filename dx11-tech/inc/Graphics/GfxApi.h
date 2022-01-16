#pragma once
#include "Graphics/GfxCommon.h"

#include <unordered_map>
#include <memory>

/*
	
	Working with Texture and Buffers in such a manner in this API puts the focus on the views.
	This allows flexibility, for example:
		- One large single underlying buffer which then uses 10 different Structured views to access different parts of the underlying buffer in isolation.

*/

class GfxApi
{
public:
	/*
		Creation
	*/
	ShaderHandle create_shader_program(const std::filesystem::path& vs_path,
		const std::filesystem::path& ps_path,
		const std::filesystem::path& gs_path = "",
		const std::filesystem::path& hs_path = "",
		const std::filesystem::path& ds_path = "");

	ShaderHandle create_compute_program(const std::filesystem::path& cs_path);

	BufferHandle create_buffer(const BufferDesc& desc, const ViewDesc& view = {});
	TextureHandle create_texture(const TextureDesc& desc, const ViewDesc& view);

	// Creates buffer from either the existing underlying buffer or a copy with identical properties
	BufferHandle create_buffer(BufferHandle hdl, const ViewDesc& view = {}, bool use_underlying_resource = false);

	// Creates texture from either the existing underlying texture or a copy with identical properties
	TextureHandle create_texture(TextureHandle hdl, const ViewDesc& view, bool use_underlying_resource = false);

	/*
		Dropping resources
	*/
	void drop_shader_program(ShaderHandle hdl);
	void drop_buffer(BufferHandle hdl);
	void drop_texture(TextureHandle hdl);

public:
	GfxApi() = delete;
	GfxApi(unique_ptr<DXDevice> dev);
	~GfxApi();

	GfxApi& operator=(const GfxApi&) = delete;
	GfxApi(const GfxApi&) = delete;

private:
	unique_ptr<DXDevice> m_dev;
	
	// ID's will just chronological values
	uint64_t m_next_buffer_id = INVALID_INTERNAL_ID + 1;
	uint64_t m_next_texture_id = INVALID_INTERNAL_ID + 1;
	uint64_t m_next_shader_id = INVALID_INTERNAL_ID + 1;

	// List of buffers and textures allocated on the GPU
	std::unordered_map<uint64_t, unique_ptr<class DXBuffer>> m_buffers;
	std::unordered_map<uint64_t, unique_ptr<class DXTexture>> m_textures;

	// List of shaders ready to use
	std::unordered_map<uint64_t, unique_ptr<class DXShader>> m_shaders;
};


