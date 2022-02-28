#pragma once
#include "Graphics/API/GfxHandles.h"
#include "Graphics/API/GfxHelperTypes.h"

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

	GfxCommandBucket<uint8_t>* get_copy_bucket() { return &m_copy_bucket; };
	GfxCommandBucket<uint64_t>* get_opaque_bucket() { return &m_opaque_bucket; };
	GfxCommandBucket<uint32_t>* get_transparent_bucket() { return &m_transparent_bucket; };
	GfxCommandBucket<uint16_t>* get_shadow_bucket() { return &m_shadow_bucket; };
	GfxCommandBucket<uint64_t>* get_postprocess_bucket() { return &m_postprocess_bucket; };

	void begin();
	void end();

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


private:
	GfxCommandBucket<uint64_t> m_main_bucket;

	Camera* m_main_cam;

	GfxCommandBucket<uint8_t> m_copy_bucket;			// For per-frame copies and other miscellaneous copies pre-draw
	GfxCommandBucket<uint16_t> m_shadow_bucket;			// Drawing geometry for shadows
	GfxCommandBucket<uint64_t> m_opaque_bucket;			// Opaque geometry
	GfxCommandBucket<uint32_t> m_transparent_bucket;	// Transparent geometry
	GfxCommandBucket<uint64_t> m_postprocess_bucket;	// Gamma correction/tone-mapping/bloom/etc.





	// temp
	std::vector<const class Model*> m_models;


	/*
		Shader reloader ImGUI variables
	*/
	std::set<std::string> shader_filenames;
	bool do_once = true;
	const char* selected_item = "";




	// Main render technique
private:
	struct PerFrameData
	{
		DirectX::XMMATRIX view_mat, proj_mat;
	} m_cb_dat;

	// Per Object data
	struct alignas(256) CBElement
	{
		DirectX::XMMATRIX world_mat;
	};
	std::array<CBElement, 5> m_cb_elements;


	bool m_vsync = true;

	// viewports
	std::vector<D3D11_VIEWPORT> viewports;

	// cbuffer
	BufferHandle m_cb_per_frame, m_big_cb;

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


