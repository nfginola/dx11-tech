#pragma once

struct BufferID;
struct TextureID;
struct ShaderID;
enum class ShaderStage;

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
	~dx() = default;

	dx& operator=(const dx&) = delete;
	dx(const dx&) = delete;

//private:
public:
	static void init(unique_ptr<DXDevice> dev);
	static void shutdown();
	static dx* get();

	void clear_backbuffer(DirectX::XMVECTORF32 color);
	void present(bool vsync = true);

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

	/* we should replace with a generic upload_res */
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
	
	


};

/*
	Strongly typed IDs for safety
*/
struct BufferID
{
	uint64_t id;
	operator uint64_t() { return id; }
};

struct TextureID
{
	uint64_t id;
	operator uint64_t() { return id; }

};

struct ShaderID
{
	uint64_t id;
	operator uint64_t() { return id; }
};

