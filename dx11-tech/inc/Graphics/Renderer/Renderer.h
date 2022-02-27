#pragma once
#include "Graphics/API/GfxHandles.h"
#include "Graphics/API/GfxHelperTypes.h"
#include "Graphics/Renderer/DrawItem.h"

#include "Graphics/CommandBucket/GfxCommandBucket.h"

/*
	
	Master Renderer.
	The render function here will have lots of if-branches and others.

	Other type of Renderers (e.g ModelRenderer, TerrainRenderer) are expected to live here.
	We do this to learn about the dependencies between different renderers before jumping to separating them!

	e.g ModelRenderer is exposed through a Getter which we can use to submit our models.
	TerrainRenderer is similarly exposed for terrain submission.


*/
class Renderer
{
public:
	static void initialize();
	static void shutdown();

	Renderer& operator=(const Renderer&) = delete;
	Renderer(const Renderer&) = delete;

	//void submit(class ICustomDrawable* drawable);
	//void set_scene(class Scene* scene);

	void set_camera(class Camera* cam);

	void render();

	void on_resize(UINT width, UINT height);
	void on_change_resolution(UINT width, UINT height);

private:
	Renderer();
	~Renderer();

	void declare_ui();
	void declare_profiler_ui();
	void declare_shader_reloader_ui();

private:
	void create_resolution_dependent_resources(UINT width, UINT height);
	void begin();
	void end();

	// Misc
private:
	GfxCommandBucket<uint8_t> m_copy_bucket;
	GfxCommandBucket<uint64_t> m_main_bucket;

	//std::vector<ICustomDrawable*> m_custom_drawables;

	//class Scene* m_curr_scene;
	bool m_proto = true;

	// temp
	std::vector<const class Model*> m_models;
	Camera* m_main_cam;


	/*
		Shader reloader ImGUI variables
	*/
	std::set<std::string> shader_filenames;
	bool do_once = true;
	const char* selected_item = "";




	// Main render technique
private:
	bool m_vsync = true;

	std::vector<D3D11_VIEWPORT> viewports;

	struct PerFrameData
	{
		DirectX::XMMATRIX view_mat, proj_mat;
	} m_cb_dat;

	struct alignas(256) CBElement
	{
		DirectX::XMMATRIX world_mat;
	};
	std::array<CBElement, 5> m_cb_elements;		// Assumes max 500 unique matrices per frame (model meshes)

	// cbuffer
	BufferHandle m_cb_per_frame, m_big_cb, m_big_cb2;

	BufferHandle m_curr_big_cb;
	BufferHandle m_prev_big_cb;
	
	SamplerHandle def_samp, repeat_samp;
	
	// render to texture 
	TextureHandle d_32;
	TextureHandle r_tex;

	RenderPassHandle r_fb;
	PipelineHandle p;

	// render to backbuffer
	RenderPassHandle fb;
	PipelineHandle r_p;




};


