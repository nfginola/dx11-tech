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


	/*  We can encapsulate this into some class ... */
	static constexpr UINT s_averaging_frames = 300;
	std::map<std::string, std::array<float, s_averaging_frames>> m_data_times;		// GPU Frame times for each profile
	GPUProfiler::FrameData avg_gpu_time;											// Lazy initialized structure for GPU times averaging per profile
	std::array<float, s_averaging_frames> m_frame_times;							// CPU Frame times
	bool is_first = true;															// Lazy initialization checker
	/*  We can encapsulate this into some class ... */


	uint64_t m_curr_frame = 0;												


	GPUProfiler* m_profiler;
	GPUAnnotator* m_annotator;

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

