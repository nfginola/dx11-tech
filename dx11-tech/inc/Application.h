#pragma once
#include "Graphics/GfxDevice.h"
#include "FrameProfiler.h"

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

	void update();

private:
	bool m_paused = false;
	bool m_app_alive = true;

	unique_ptr<class Window> m_win;
	unique_ptr<class Input> m_input;


	std::vector<D3D11_VIEWPORT> viewports;										

	// triangle (temporary)
	GPUBuffer vb_pos;
	GPUBuffer vb_uv;
	GPUBuffer vb_nor;
	GPUBuffer ib;

	struct PerFrameData
	{
		DirectX::XMMATRIX view_mat, proj_mat;
	} m_cb_dat;

	struct alignas(256) CBElement
	{
		DirectX::XMMATRIX world_mat;
	};
	std::array<CBElement, 2> m_cb_elements;

	// cbuffer
	GPUBuffer m_cb_per_frame;
	GPUBuffer m_big_cb;

	unique_ptr<class Camera> m_cam;

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

	uint64_t m_curr_frame = 0;




};

