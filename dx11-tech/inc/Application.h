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

