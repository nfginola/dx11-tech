#pragma once
#include "Graphics/GfxDevice.h"
#include "Graphics/ImGuiDevice.h"
#include "Profiler/FrameProfiler.h"

#include "Graphics/Model.h"
#include "AssimpLoader.h"


class Application
{
public:
	Application();
	~Application();

	Application& operator=(const Application&) = delete;
	Application(const Application&) = delete;

	void run();

private:
	LRESULT custom_win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void load_assets();

	void declare_ui();
	void declare_profiler_ui();
	void declare_shader_reloader_ui();

	void create_resolution_dependent_resources(UINT width, UINT height);
	void on_resize(UINT width, UINT height);

	void update(float dt);

private:
	bool m_paused = false;
	bool m_app_alive = true;
	bool m_vsync = true;

	unique_ptr<class Window> m_win;
	unique_ptr<class Input> m_input;

	std::vector<D3D11_VIEWPORT> viewports;										

	std::vector<Material> m_materials;
	// sponza
	Model m_sponza;
	GPUBuffer sp_vb_pos;
	GPUBuffer sp_vb_uv;
	GPUBuffer sp_vb_nor;
	GPUBuffer sp_ib;
	std::vector<AssimpMeshData> m_sp_meshes;
	std::vector<GPUTexture*> m_sp_textures;

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

	unique_ptr<class FPCController> m_camera_controller;
	unique_ptr<class FPPCamera> m_cam;

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

	uint64_t m_curr_frame = 0;

	bool m_resize_allowed = false;
	bool m_should_resize = false;
	std::pair<UINT, UINT> m_resized_client_area;



	/*
		Shader reloader variables
	*/
	std::set<std::string> shader_filenames;
	bool do_once = true;
	const char* selected_item = "";



};

