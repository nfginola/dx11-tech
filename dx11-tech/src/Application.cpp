#include "pch.h"
#include "Application.h"
#include "Window.h"
#include "Input.h"
#include "Timer.h"

#include "Camera/FPCController.h"
#include "Camera/FPPCamera.h"

#include "Graphics/DiskTextureManager.h"
#include "Graphics/MaterialManager.h"
#include "Graphics/ModelManager.h"
#include "Graphics/Model.h"
#include "Graphics/Renderer.h"

// Important that Globals is defined last, as the extern members need to be defined!
// We can define GfxGlobals.h if we want to have a separation layer later 
// Maybe we would like to have like WickedEngine where we have a Renderer.cpp and Renderer.hpp with all static functions
// And only allow GfxGlobals in that Renderer.cpp
// Just an idea.
#include "Globals.h"

#include "Graphics/Drawable/CustomTestDrawable.h"


Application::Application()
{
	// Window render area dimension
	constexpr UINT WIN_WIDTH = 1920;
	constexpr UINT WIN_HEIGHT = 1080;

	// Resolution
	constexpr UINT WIDTH = WIN_WIDTH;
	constexpr UINT HEIGHT = WIN_HEIGHT;

	// Window and Input
	auto win_proc = [this](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT { return this->custom_win_proc(hwnd, uMsg, wParam, lParam); };
	m_win = make_unique<Window>(GetModuleHandle(NULL), win_proc, WIN_WIDTH, WIN_HEIGHT);
	m_input = make_unique<Input>(m_win->get_hwnd());

	// Initialize singletons (with clear dependencies)
	GfxDevice::initialize(make_unique<DXDevice>(m_win->get_hwnd(), WIDTH, HEIGHT));
	FrameProfiler::initialize(make_unique<CPUProfiler>(), gfx::dev->get_profiler());
	ImGuiDevice::initialize(gfx::dev);

	DiskTextureManager::initialize(gfx::dev);
	MaterialManager::initialize(gfx::tex_mgr);
	ModelManager::initialize(gfx::dev, gfx::mat_mgr);
	
	Renderer::initialize();

	// Declare UI 
	ImGuiDevice::add_ui("default ui", [&]() { declare_ui(); });
	ImGuiDevice::add_ui("profiler", [&]() { declare_profiler_ui();  });
	ImGuiDevice::add_ui("shader reloading", [&]() { declare_shader_reloader_ui();  });

	// Create perspective camera
	m_cam = make_unique<FPPCamera>(90.f, (float)WIDTH/HEIGHT, 0.1f, 1000.f);
	m_cam_zoom = make_unique<FPPCamera>(28.f, (float)WIDTH / HEIGHT, 0.1f, 1000.f);	// zoomed in
	
	// Create a First-Person Camera Controller and attach a First-Person Perspective camera
	m_camera_controller = make_unique<FPCController>(m_input.get());
	m_camera_controller->set_camera(m_cam.get());
	m_camera_controller->set_secondary_camera(m_cam_zoom.get());

}

Application::~Application()
{
	Renderer::shutdown();
	ModelManager::shutdown();
	MaterialManager::shutdown();
	DiskTextureManager::shutdown();
	ImGuiDevice::shutdown();
	FrameProfiler::shutdown();
	GfxDevice::shutdown();
}

void Application::run()
{
	float dt = 0.f;
	while (m_win->is_alive() && m_app_alive)
	{
		perf::profiler->frame_start();
		Timer frame_time;

		// Block here if paused
		while (m_paused);
		m_win->pump_messages();

		m_input->begin();
			
		// Update CPU states
		update(dt);
	
		// Render GPU
		gfx::rend->set_camera(m_camera_controller->get_active_camera());
		gfx::rend->render();

		m_input->end();

		++m_curr_frame;
		dt = frame_time.elapsed(Timer::Unit::Seconds);
		perf::profiler->frame_end();
	
		// Resize once at the end of a frame
		// Avoid resizing constantly
		if (m_should_resize)
		{
			on_resize(m_resized_client_area.first, m_resized_client_area.second);
			m_should_resize = false;
		}
	}
}

void Application::declare_ui()
{
	ImGui::Begin("Settings");
	ImGui::Checkbox("Vsync", &m_vsync);
	ImGui::End();


	// Declare things to draw UI overlay
	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
	ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
	ImGui::Checkbox("Another Window", &show_demo_window);

	ImGui::SameLine();

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	// Main menu
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) { fmt::print("Undid!\n"); }
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

	}

	ImGui::Begin("Settings");

	const char* items[] = { "2560x1440", "1920x1080", "1280x720", "640x360", "384x216" };
	static int item_current_idx = 0; // Here we store our selection data as an index.
	const char* combo_preview_value = items[item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
	bool change_res = false;
	if (ImGui::BeginCombo("Resolutions", combo_preview_value))
	{
		for (int n = 0; n < IM_ARRAYSIZE(items); n++)
		{
			const bool is_selected = (item_current_idx == n);
			if (ImGui::Selectable(items[n], is_selected))
			{
				item_current_idx = n;
				change_res = true;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if (change_res)
	{
		if (item_current_idx == 0)		gfx::rend->on_change_resolution(2560, 1440);
		if (item_current_idx == 1)		gfx::rend->on_change_resolution(1920, 1080);
		if (item_current_idx == 2)		gfx::rend->on_change_resolution(1280, 720);
		if (item_current_idx == 3)		gfx::rend->on_change_resolution(640, 360);
		if (item_current_idx == 4)		gfx::rend->on_change_resolution(384, 216);
	}

	ImGui::End();

}

void Application::declare_profiler_ui()
{
	bool open = true;
	ImGui::Begin(fmt::format("Times (avg. over {} frames)", FrameProfiler::s_averaging_frames).c_str(), &open, ImGuiWindowFlags_NoTitleBar);

	const auto& frame_data = perf::profiler->get_frame_statistics();

	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("CPU"))
	{
		for (const auto& profiles : frame_data.profiles)
		{
			const auto& name = profiles.first;
			const auto& avg_cpu_time = profiles.second.avg_cpu_time;
			if (abs(avg_cpu_time) <= std::numeric_limits<float>::epsilon())		// skip negligible profiles
				continue;

			ImGui::Text(fmt::format("{:s}: {:.3f} ms", name.c_str(), avg_cpu_time).c_str());
		}
		ImGui::TreePop();
	}

	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("GPU"))
	{
		for (const auto& profiles : frame_data.profiles)
		{
			const auto& name = profiles.first;
			const auto& avg_gpu_time = profiles.second.avg_gpu_time;
			if (abs(avg_gpu_time) <= std::numeric_limits<float>::epsilon())		// skip negligible profiles
				continue;

			ImGui::Text(fmt::format("{:s}: {:.3f} ms", name.c_str(), avg_gpu_time).c_str());
		}
		ImGui::TreePop();
	}

	// Get FPS
	const auto& full_frame = frame_data.profiles.find("*** Full Frame ***");
	if (full_frame != frame_data.profiles.cend())
	{
		auto avg_frame_time = (full_frame->second.avg_cpu_time + full_frame->second.avg_gpu_time) / 2.f;
		avg_frame_time /= 1000.f;
		ImGui::Text(fmt::format("FPS: {:.1f}", 1.f / avg_frame_time).c_str());
	}

	ImGui::End();
}

void Application::declare_shader_reloader_ui()
{
	if (do_once)
	{
		const std::string path = "shaders";
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (std::filesystem::path(entry).extension() == ".hlsli")	// discard hlsli
				continue;

			shader_filenames.insert(std::filesystem::path(entry).stem().string());

		}
		selected_item = shader_filenames.cbegin()->c_str();
		do_once = false;
	}

	ImGui::Begin("Shader Reloader");

	const char* combo_preview_value = selected_item;
	if (ImGui::BeginCombo("Files", combo_preview_value))
	{
		for (const auto& filename : shader_filenames)
		{
			const bool is_selected = strcmp(selected_item, filename.c_str());

			if (ImGui::Selectable(filename.c_str(), is_selected))
			{
				selected_item = filename.c_str();
				combo_preview_value = selected_item;
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}
	if (ImGui::SmallButton("Reload"))
		gfx::dev->recompile_pipeline_shaders_by_name(std::string(selected_item));

	ImGui::End();
}

void Application::on_resize(UINT width, UINT height)
{
	if (width == m_win->get_client_width() && height == m_win->get_client_height())
		return;

	fmt::print("resize with dimensions: [Width: {}], [Height: {}]\n", m_resized_client_area.first, m_resized_client_area.second);

	m_win->resize_client(width, height);
	gfx::dev->resize_swapchain(width, height);
	gfx::rend->on_resize(width, height);
}

void Application::update(float dt)
{
	// Update camera controller
	m_camera_controller->update(dt);

}

LRESULT Application::custom_win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	gfx::imgui->win_proc(hwnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
		// DirectXTK Mouse and Keyboard (Input)
	case WM_ACTIVATEAPP:
	{
		if (m_input)
		{
			m_input->process_keyboard(uMsg, wParam, lParam);
			m_input->process_mouse(uMsg, wParam, lParam);
		}
	}
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
	{
		if (m_input)
			m_input->process_mouse(uMsg, wParam, lParam);

		break;
	}

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			m_app_alive = false;
		}
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (m_input)
			m_input->process_keyboard(uMsg, wParam, lParam);
		break;

	
		// Only called on pressing "X"
	case WM_CLOSE:
	{
		//std::cout << "what\n";
		//KillApp();
		break;
	}

	// Resize message
	case WM_SIZE:
	{
		/*
			This is called frequently when resizing happens.
			Hence why m_allow_resize is used so that we can defer the operations until later
		*/
		m_resize_allowed = true;
		m_resized_client_area.first = LOWORD(lParam);
		m_resized_client_area.second = HIWORD(lParam);
		break;
	}
	case WM_ENTERSIZEMOVE:
	{
		m_paused = true;
		break;
	}
	case WM_EXITSIZEMOVE:
	{
		m_paused = false;

		if (m_resize_allowed)
		{
			m_should_resize = true;
			m_resize_allowed = false;
		}
		break;
	}

	case WM_SYSKEYDOWN:
	{
		// Custom Alt + Enter to toggle windowed borderless (disabled for now)
		/*
			DO NOT DRAG IMGUI WINDOWS IN FULLSCREEN!
		*/
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{
			m_win->set_fullscreen(!m_win->is_fullscreen());
			if (m_resize_allowed)
			{
				m_should_resize = true;
				m_resize_allowed = false;
			}
		}
		break;
	}

	default:
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

