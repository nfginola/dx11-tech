#pragma once
#include "Graphics/API/GfxDevice.h"
#include "Graphics/API/ImGuiDevice.h"
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

	void on_resize(UINT width, UINT height);

	void update(float dt);

private:
	bool m_paused = false;
	bool m_app_alive = true;
	bool m_vsync = true;

	unique_ptr<class Window> m_win;
	unique_ptr<class Input> m_input;

	unique_ptr<class FPCController> m_camera_controller;
	unique_ptr<class FPPCamera> m_cam, m_cam_zoom;


	bool m_resize_allowed = false;
	bool m_should_resize = false;
	std::pair<UINT, UINT> m_resized_client_area;

	uint64_t m_curr_frame = 0;



	class IDrawable* m_drawable;


};

