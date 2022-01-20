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
	Framebuffer fb;
	GraphicsPipeline p;
	std::vector<D3D11_VIEWPORT> viewports = { CD3D11_VIEWPORT(0.f, 0.f, 1920.f, 1080.f) };

	Framebuffer r_fb;

	bool m_paused = false;
	bool m_app_alive = true;

	unique_ptr<class Window> m_win;
	unique_ptr<class Input> m_input;

	LRESULT custom_win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

};

