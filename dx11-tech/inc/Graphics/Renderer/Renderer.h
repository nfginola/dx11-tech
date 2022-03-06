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
class GfxDevice;

/*
	Resources that may be needed for the renderers which submit draw data.
*/
struct RendererSharedResources
{
	// Common
	PipelineHandle depth_only_pipe;

	// Deferred specific
	PipelineHandle deferred_gpass_pipe;

	TextureHandle temp_depth;
};

class Renderer
{
public:
	static void initialize();
	static void shutdown();

	Renderer& operator=(const Renderer&) = delete;
	Renderer(const Renderer&) = delete;
	
	/*
		If possible, the buckets exposed should be independent of the underlying primary technique (Deferred/Forward+)
		This way, we can have a Renderer interface and swap between different techniques! (and extend in a flexible manner)
	*/
	GfxCommandBucket<uint8_t>* get_copy_bucket() { return &m_copy_bucket; };
	GfxCommandBucket<uint8_t>* get_compute_bucket() { return &m_compute_bucket; };
	GfxCommandBucket<uint64_t>* get_opaque_bucket() { return &m_opaque_bucket; };
	GfxCommandBucket<uint32_t>* get_transparent_bucket() { return &m_transparent_bucket; };
	GfxCommandBucket<uint16_t>* get_shadow_bucket() { return &m_shadow_bucket; };
	GfxCommandBucket<uint64_t>* get_postprocess_bucket() { return &m_postprocess_bucket; };

	// Non-owning pointer to shared resources
	// Should these resources be updated internally, all other modules using it will have the changes reflected appropriately
	const RendererSharedResources* get_shared_resources() { return &m_shared_resources; };

	void begin();
	void end();

	void set_camera(class Camera* cam);

	void render();

	void on_resize(UINT width, UINT height);
	void on_change_resolution(UINT width, UINT height);



private:
	Renderer();
	~Renderer() = default;

	void declare_ui();
	void declare_profiler_ui();
	void declare_shader_reloader_ui();

private:
	void create_resolution_dependent_resources(UINT width, UINT height);

private:
	struct PerFrameData
	{
		DirectX::XMMATRIX view_mat, proj_mat;
	} m_cb_dat;

	// Phong GBuffer
	struct GBuffer
	{
		TextureHandle albedo;
		TextureHandle normal;
		TextureHandle world;

		RenderPassHandle rp;
		
		// utilities
		void create_gbuffers(GfxDevice* dev, UINT width, UINT height);
		void create_rp(GfxDevice* dev, TextureHandle depth);
		void read_bind(GfxDevice* dev);
		void free(GfxDevice* dev);
	};

private:
	bool m_vsync = true;


	// Main render technique
private:
	Camera* m_main_cam;

	// Command buckets for dispatches
	GfxCommandBucket<uint8_t> m_copy_bucket;			// For per-frame copies and other miscellaneous copies pre-draw
	GfxCommandBucket<uint8_t> m_compute_bucket;			// GPU compute work
	GfxCommandBucket<uint16_t> m_shadow_bucket;			// Drawing geometry for shadows
	GfxCommandBucket<uint64_t> m_opaque_bucket;			// Opaque geometry
	GfxCommandBucket<uint32_t> m_transparent_bucket;	// Transparent geometry
	GfxCommandBucket<uint64_t> m_postprocess_bucket;	// Gamma correction/tone-mapping/bloom/etc.

	RendererSharedResources m_shared_resources;

	/*
		Deferred rendering
	*/

	// Geometry pass
	bool m_allocated = false;	// Resolution-dependent resources check
	TextureHandle m_d_32;			// 32-bit depth
	GBuffer m_gbuffer_res;

	// Shadow Pass
	TextureHandle m_dir_d32;
	RenderPassHandle m_dir_rp;
	SamplerHandle m_shadow_sampler;
	//PipelineHandle m_depth_only_pipe;


	// temporary (should be moved to a shadow mapper)
	/*
		MasterRenderer owns a ShadowMapper
			--> ShadowMapper owns resources related to shadow mapping, but the master renderer can look into it and use exposed resources
		
	*/
	struct PerLightData
	{
		DirectX::XMMATRIX view_proj;
		DirectX::XMMATRIX view_proj_inv;
		DirectX::XMFLOAT4 light_direction;
	};
	PerLightData m_light_data;
	BufferHandle m_per_light_cb;


	// Light-pass
	TextureHandle m_lightpass_output;
	PipelineHandle m_lightpass_pipe;
	RenderPassHandle m_lightpass_rp;

	// Final post-processs (e.g gamma correction / tonemapping)
	PipelineHandle m_final_pipe;
	RenderPassHandle m_backbuffer_out_rp;

	/*
		Other
	*/
	
	// CBuffer for per-frame update frequency
	BufferHandle m_cb_per_frame;
	
	// Persistent samplers (we should make a struct for that)
	SamplerHandle def_samp, repeat_samp;

	// viewports
	std::vector<D3D11_VIEWPORT> viewports;

	// Miscellaneous 
private:

	// Shader reloader ImGUI variables
	std::set<std::string> shader_filenames;
	bool do_once = true;
	const char* selected_item = "";




};


