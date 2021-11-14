#pragma once
#include "Graphics/DXCommon.h"

/*

	Singleton API for Direct3D11 for ease of use

*/
class dx
{
private:
	// friend class PipelineState
	// We let PipelineState get access to privates so that it can use resource defaults on creation

	/*
		friend class Application;
	
		We can limit the availability of this singleton through friends so that it
		is not accidentally accessed in other parts of the program where we dont want
	*/

	static dx* s_self;
	unique_ptr<class DXDevice> m_dev;

	PipelineStateID m_def_pipeline;


	/*
	
	// ID is literally the index into the vector.
	// free slots in queue lets use re-use slots that have been freed

	std::vector<std::pair<BufferID, DXBuffer>> m_buffers;
	std::vector<std::pair<TextureID, DXTexture>> m_textures;
	std::vector<std::pair<TextureID, DXShader>> m_shaders;

	std::queue<BufferID> m_free_buffer_slots;
	std::queue<BufferID> m_free_texture_slots;

	PipelineState* m_bound_state;	// used so we can do member-wise comparison to avoid binding already bound state

	
	*/

public:
	dx(unique_ptr<DXDevice> dev);
	~dx();

	dx& operator=(const dx&) = delete;
	dx(const dx&) = delete;

	/* 
		Static defaults for each PipelineDescriptor member

		dx::def_topology()
		dx::def_rasterizer()
		dx::def_...
	*/

//private:
public:
	static void init(unique_ptr<DXDevice> dev);
	static void shutdown();
	static dx* get();

	void clear_backbuffer(DirectX::XMVECTORF32 color);
	void present(bool vsync = true);
	/*
	void clear_fbo(...)
	*/

	// Resource creation
	/*
	
		we could just mimic D3D11 API or make functions that make creation explicit

		e.g create_texture1d/2d/3d (manual structs) vs create_texture(d3d11 structs)
	
		easiest would be to mimic d3d11 desc but for most learning, we would want to have explicit:
			- create_rw_buffer();

			- create_shader_prog(vs, ps, gs, hs, ds);
			- create_shader_compute(..)
	
		should we try using std::unordered_map and return IDs instead? :)

		shaderProgram can use filepaths to hash 

	*/
	BufferID create_vertex_buffer();
	BufferID create_index_buffer();
	BufferID create_buffer();
	TextureID create_texture();
	ShaderID create_shader(
		const std::filesystem::path& vs_path,
		const std::filesystem::path& ps_path,
		const std::filesystem::path& hs_path = "",
		const std::filesystem::path& ds_path = "",
		const std::filesystem::path& gs_path = "");

	PipelineStateID create_pipeline();

	/*
	
	// getters so we can query data
	// such as buffer size, element size, texture dims/settings, etc.
	// idea here is to have it easily replaceable with some Buffer/Texture abstraction that is common
	// across many APIs, but here, we will only use DX11.
	// but add this LATER IF we do see the need to use it (dont expose API prematurely)
	// why? because at the place where 

	DXBuffer* get_buffer(id);
	DXTexture* get_texture(id);
	
	*/

	void hot_reload_shader(ShaderID id);

	/* we should internally use a generic upload_res */
	// at the interface level, we can have two different:
	/*
		- we should somehow figure out when to choose map/unmap or updatesubres accordingly
			- maybe let the user assign some "Frequency" enum: 
				- e.g "UpdateFrequency::Often" or "UpdateFrequency::Sometimes" 
				- there are also other factors, like the fact that Texture will always use UpdateSubresource (has box rect and all that stuff)
		upload_to_buffer
		upload_to_texture
	*/
	void upload_to_buffer(void* data, uint64_t size, BufferID id);

	// Resource destruction
	/*
	void release_buffer(BufferID id);
	void release_texture(TextureID id);
	*/

	/*
	* 
	Buffer* get_buffer(BufferID id);
	Texture* get_texture(TextureID id);
	
	*/
	
	// Resource binding
	void bind_buffer(uint8_t slot, ShaderStage stage, BufferID id);
	void bind_vertex_buffer(BufferID id);
	void bind_index_buffer(BufferID id);
	void bind_texture(uint8_t slot, ShaderStage stage, TextureID id);
	void bind_pipeline(PipelineStateID id);


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
			DXShaderID [1]
		}

		---- example code (deferred), but lets implement Forward+!
		Framebuffer fbo;
		
		bind_fbo_output(fbo);
		for each single_draw_call:					(no need to sort for now)
			set_pipeline(pipeline);
			draw geometry to gbuffer
		for each instanced_draw_call:
			set_pipeline(pipeline)
			draw geometry to gbuffer
		unbind_fbo_output(fbo);

		bind_fbo_input(fbo);
		set_pipeline(pipeline2);
		draw fullscreen quad
		unbind_fbo_input(fbo);


		
		struct DrawCall
		{
			BuffferID vb
			BufferID ib
			PipelineStateID pipeline
		}

		void submit_draw(DrawCall);
	*/
	
private:
	void create_default_resources();

	


};



