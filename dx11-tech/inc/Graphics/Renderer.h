#pragma once
#include "Graphics/GfxTypes.h"

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

private:
	Renderer();
	~Renderer();

private:
	void create_resolution_dependent_resources(UINT width, UINT height);

	// Misc
private:
	//std::vector<ICustomDrawable*> m_custom_drawables;

	//class Scene* m_curr_scene;
	
	// temp
	std::vector<const class Model*> m_models;
	Camera* m_main_cam;













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
	std::array<CBElement, 1> m_cb_elements;

	// cbuffer
	GPUBuffer m_cb_per_frame;
	GPUBuffer m_big_cb;

	// render to texture 
	GPUTexture d_32;
	GPUTexture r_tex;
	Framebuffer r_fb;
	GraphicsPipeline p;

	// render to backbuffer
	Framebuffer fb;
	GraphicsPipeline r_p;

	// linear minmagmip
	Sampler def_samp;
	Sampler repeat_samp;



};

