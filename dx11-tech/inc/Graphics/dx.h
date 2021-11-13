#pragma once

struct BufferID;
struct TextureID;

/*

	Singleton API for Direct3D11 for ease of use

*/
class dx
{
private:
	/*
		friend class Application;
	
		We can limit the availability of this singleton through friends so that it
		is not accidentally accessed in other parts of the program where we dont want
	*/

	static dx* s_self;
	shared_ptr<class DXDevice> m_dev;

	/*
	
	// ID is literally the index into the vector.
	// free slots in queue lets use re-use slots that have been freed

	std::vector<std::pair<BufferID, Buffer>> m_buffers;
	std::vector<std::pair<TextureID, Texture>> m_textures;

	std::queue<BufferID> m_free_buffer_slots;
	std::queue<BufferID> m_free_texture_slots;

	
	*/

public:
	dx(shared_ptr<DXDevice> dev);
	~dx() = default;

	dx& operator=(const dx&) = delete;
	dx(const dx&) = delete;

//private:
public:
	static void init(shared_ptr<DXDevice> dev);
	static void shutdown();
	static dx* get();

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

	void bind_buf(BufferID id);
	void bind_tex(TextureID id);

	/*
	void release_buffer(BufferID);
	void release_texture(TextureID);
	
	*/



	// State setting and submission
	/*
	
		Do we want to mimic D3D11 state machine interface?
		Or do we want to abstract to sending in a State struct? (essentially just like a PipelineState or something similar which has all state info?)
	
	*/

	/*
	struct PipelineState
	{
		ShaderProgram
		... read vulkan


	}
	
	
	
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

