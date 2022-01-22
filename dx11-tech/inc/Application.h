#pragma once

#include "Graphics/GfxDevice.h"


class Application
{
public:
	Application();
	~Application();

	Application& operator=(const Application&) = delete;
	Application(const Application&) = delete;

	void run();

private:
	std::vector<D3D11_VIEWPORT> viewports;

	GPUBuffer vb_pos;
	GPUBuffer vb_uv;
	GPUBuffer vb_nor;
	GPUBuffer ib;

	GPUTexture d_32;

	Framebuffer r_fb;		
	GPUTexture r_tex_ms;	// render to texture multi-sample
	GPUTexture r_tex;		// render to texture resolve
	GraphicsPipeline p;		

	Framebuffer fb;			// render to backbuffer
	GraphicsPipeline r_p;

	Sampler def_samp;		// linear minmagmip

	bool m_paused = false;
	bool m_app_alive = true;

	unique_ptr<class Window> m_win;
	unique_ptr<class Input> m_input;

	LRESULT custom_win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

};

