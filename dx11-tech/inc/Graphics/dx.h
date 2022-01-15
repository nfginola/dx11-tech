#pragma once
#include "Graphics/GfxCommon.h"
#include <queue>

class dx
{
private:
	static dx* s_self;
	unique_ptr<class DXDevice> m_dev;

	PipelineHandle m_def_pipeline;

	/*
	
	// ID is literally the index into the vector.
	// free slots in queue lets use re-use slots that have been freed

	std::vector<std::pair<BufferHandle, DXBuffer>> m_buffers;
	std::vector<std::pair<TextureHandle, DXTexture>> m_textures;
	std::vector<std::pair<TextureHandle, DXShader>> m_shaders;

	std::queue<BufferHandle> m_free_buffer_slots;
	std::queue<BufferHandle> m_free_texture_slots;

	PipelineState* m_bound_state;	// used so we can do member-wise comparison to avoid binding already bound state

	*/

public:
	dx(unique_ptr<DXDevice> dev);
	~dx();

	dx& operator=(const dx&) = delete;
	dx(const dx&) = delete;

public:
	static void init(unique_ptr<DXDevice> dev);
	static void shutdown();
	static dx* get();

	void start_frame();
	void end_frame();


	void clear_backbuffer(DirectX::XMVECTORF32 color);
	void present(bool vsync = true);
	/*
	void clear_fbo(...)
	*/
	BufferHandle create_vertex_buffer();
	BufferHandle create_index_buffer();

	
	BufferHandle create_buffer();

	TextureHandle create_texture();
	ShaderHandle create_shader(
		const std::filesystem::path& vs_path,
		const std::filesystem::path& ps_path,
		const std::filesystem::path& hs_path = "",
		const std::filesystem::path& ds_path = "",
		const std::filesystem::path& gs_path = "");

	ShaderHandle create_compute_shader(const std::filesystem::path& cs_path);

	PipelineHandle create_pipeline();


	void hot_reload_shader(ShaderHandle handle);

	/* we should internally use a generic upload_res */
	// at the interface level, we can have two different:
	/*
		- we should somehow figure out when to choose map/unmap or updatesubres accordingly
			- maybe let the user assign some "Frequency" enum: 
				- there are also other factors, like the fact that Texture will always use UpdateSubresource (has box rect and all that stuff)
				- e.g "UpdateFrequency::Often" or "UpdateFrequency::Sometimes" 

		upload_to_buffer
		upload_to_texture
	*/
	void upload_to_buffer(void* data, uint64_t size, BufferHandle handle);



	
	// Resource binding
	void bind_buffer(uint8_t slot, BAccess mode, ShaderStage stage, BufferHandle handle);
	void bind_vertex_buffer(BufferHandle handle);
	void bind_index_buffer(BufferHandle handle);
	void bind_texture(uint8_t slot, TAccess mode, ShaderStage stage, TextureHandle handle);
	void bind_pipeline(PipelineHandle handle);
	void bind_shader(ShaderHandle handle);

	/*
		We can replace with some common geometries:
			- draw_common(CommonGeometry::Triangle);
			- draw_common(CommonGeometry::FullscreenQuad);
			- draw_common(CommonGeometry::Skybox);		
			- draw_common(CommonGeometry::Plane);
			- draw_common(CommonGeometry::Cube);
			- draw_common(CommonGeometry::Sphere);	// We can try our hands at Compute Shader generated Sphere through polar coordinates
	*/
	void draw_fullscreen_quad();
	
	// Pipeline state setting
	/*
	
		Do we want to mimic D3D11 state machine interface?
		Or do we want to abstract to sending in a State struct? (essentially just like a PipelineState or something similar which has all state info?)

		struct PipelineState
		{
			// IA
			Topology [1]						(VkPipelineInputAssembly)
			Input Layout [1]					(VkPipelineVertexInputState)

			// RS
			RS Viewport and RS Scissors [0, n]
			Rasterizer State [1]

			// OM
			Depth Stencil State [1]
			Blend State [1]
			
			// Shader Program
			DXShaderHandle [1]
		}

		---- example code (deferred), but lets implement Forward+!
		Framebuffer fbo;
		
		bind_fbo_output(fbo);
		for each single_draw_call:				
			set_pipeline(pipeline);
			draw geometry to gbuffer
		for each instanced_draw_call:
			set_pipeline(pipeline)
			--> here we set a predefined buffer for instance data with a defined max size and reuse it
			draw geometry to gbuffer
		unbind_fbo_output(fbo);

		bind_fbo_input(fbo);
		set_pipeline(pipeline2);
		draw fullscreen quad
		unbind_fbo_input(fbo);
		
		It is the responsibility of the API user to sort the "DRAW CALLS" on their own using some method.
		The handles are not meant to be used as numbers to sort.

		Sorting should be done on a higher level (Materials, Transparency, etc.)
		--> On that higher level, if for example sorted by Materials..
			--> on set_pipeline, the API will see that stuff is already bound, hence less GPU binds!

	*/
	
private:
	void create_default_resources();

	void unbind_writes_with_uav(ShaderStage stage, uint32_t slot);
	void unbind_rtvs_dsv();

	


};



