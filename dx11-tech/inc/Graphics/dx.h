#pragma once

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
	void create_vertex_buffer();
	void create_index_buffer();
	void create_buffer();
	void create_texture();

	// State setting and submission
	/*
	
		Do we want to mimic D3D11 state machine interface?
		Or do we want to abstract to sending in a State struct? (essentially just like a PipelineState or something similar which has all state info?)
	
	*/

};

