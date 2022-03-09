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
#include "Graphics/Renderer/Renderer.h"

// Important that Globals is defined last, as the extern members need to be defined!
// We can define GfxGlobals.h if we want to have a separation layer later 
// Maybe we would like to have like WickedEngine where we have a Renderer.cpp and Renderer.hpp with all static functions
// And only allow GfxGlobals in that Renderer.cpp
// Just an idea.
#include "Globals.h"

Application::Application()
{
	/*

		To-do:
			- Add raster UAVs for OMSetRenderTargetsAndUAV			DONE
			- Add VB/IB binding to API								DONE
			- Draw triangle with VB/IB								DONE (Non-interleaved data too!)
			- Add HDR rendering and tone mapping					DONE (ACES tonemapping added)
			- Enable multisampling									DONE
			- Add Set Resource Naming								DONE
			- Add Set/End EventMarker? 11.3 (What is that)			DONE
				- We used the ID3DUserDefinedAnnotation!			DONE
			- Add GPU Queries for Timestamp and Pipeline Stats		DONE
				- Add a GetData() to retrieve useful data			DONE
				- Add time averaging for timestamp and CPU			DONE
					- Over 500 frames?

			- Add shader hot reloading!								DONE
				- Do this through pipeline hot reloading
				- Check comment below by the input code

			- Encapsulate Profiler									DONE
				- Averaging takes time, what to do?					NEGLIGIBLE, CHECK TIMER IN FrameProfiler

			- Use fmt for printing									DONE (linked as a VS Project dependency)
				- https://github.com/fmtlib/fmt

			- Add Map and UpdateSubresource							DONE

			- Add Perspective Camera (normal depth, no reversed)	DONE

			- Add moving and looking around							DONE

			- Refactor camera using a FP Controller					DONE

			- Add ImGUI docking branch								DONE
				- https://github.com/ocornut/imgui/wiki/Docking

			- Set Backbuffer to a Dockable Render Target?			DONE

			- Refactor the way we submit ImGUI code					DONE
				- Add to global callback list
				  which is all run before UI draw

			- Use ImGUI bar to show Frame Statistics				DONE
				X Bars for Full Frame
				X Numbers in ms on the side for parts
				+ We just print numbers instead to keep it simple

			- Add shader reloader GUI								DONE
				- Directory auto scanning
				- Filters out .hlsli

			- Add Assimp Loader										DONE

			- Refactor dependencies									DONE
				- Lib linking and minimal files

			- Add TextureManager									DONE
				- Read notes in load_assets()

			- Load Sponza Textures									DONE

			- Fix auto-gen mipmaps									DONE
				- Using UpdateSubresource to do
				  staging copy

			- Add a Model class										DONE
				- Simply has Meshes and Materials (1:1 mapping)
				- Later down the line, we want to reformat for
					instancing.
				- Make a naive version for now

			- Add Material repository								DONE

			- Add model repository									DONE
				- Ignore MT contention problems

			- Add Pipeline cache									DONE
				- Check prev_pipeline
				- Also, use prev_ps to determine whether or not to
				  bind the new VS AND Input Layout!
				  They are connected!
					- Even though we duplicate Input Layouts
					  We can avoid binding the same ones
					  at least.

			- Create a Pipeline Manager								TO-DO
				- Create Pipeline (naming too)
				- Retrieve Pipeline

			- Add a Simple Entity which holds a World Matrix		TO-DO
				- Holds a pointer to an existing Model (Flyweight)

			- Implement rotation using Quaternion					TO-DO

			- Add instancing by default								TO-DO
				- Renderer.SubmitOpaque(model, world_mat, FLAGS)

			FLAGS |= visible

			for each unique model submitted,
			we allocate an array for world matrices PER MODEL
				--> serves as instance buffer

			we submit DrawData:
				{ mesh*, material*, vbs*, strides*, ib*, InstanceData { void*, size } }
			--> sort DrawData by Material first and Index Buffer second




			- Add AABBs to Entities									TO-DO
				- Models have local AABB
				- Entities receive a copy that is
				  world-adjusted to the specific Entity

			- Frustum Culling										TO-DO
				-	AABBs

			- Octtree?												TO-DO

			- Try recreating view-space positions from only depth!	TO-DO

			- Bind Persistent Samplers (on the last slots stages)	TO-DO
				- Check MJP samples and DXTK for Common Samplers
				- Remember that shadows use diff. samplers

			- Create Swap chain class?								TO-DO
				- Refactoring work

			- Grab Pipeline Query									TO-DO
				- Refactoring work

			- Check out JSON format and see if it is useful			TO-DO
				- Check out nlohmanns JSON parser

	


	*/



	// Window render area dimension
	constexpr UINT WIN_WIDTH = 1920;
	constexpr UINT WIN_HEIGHT = 1080;

	// Resolution
	constexpr UINT WIDTH = WIN_WIDTH;
	constexpr UINT HEIGHT = WIN_HEIGHT;

	// Initialize window and input
	auto win_proc = [this](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT { return this->custom_win_proc(hwnd, uMsg, wParam, lParam); };
	m_win = make_unique<Window>(GetModuleHandle(NULL), win_proc, WIN_WIDTH, WIN_HEIGHT);
	m_input = make_unique<Input>(m_win->get_hwnd());


	// Initialize systems
	CPUProfiler::initialize();
	GfxDevice::initialize(make_unique<DXDevice>(m_win->get_hwnd(), WIDTH, HEIGHT));
	FrameProfiler::initialize(perf::cpu_profiler, gfx::dev->get_profiler());
	ImGuiDevice::initialize(gfx::dev);

	DiskTextureManager::initialize(gfx::dev);
	MaterialManager::initialize(gfx::tex_mgr);
	ModelManager::initialize(gfx::dev, gfx::mat_mgr);
	
	Renderer::initialize();

	// Create perspective camera
	m_cam = make_unique<FPPCamera>(90.f, (float)WIDTH/HEIGHT, 0.1f, 600.f);
	m_cam_zoom = make_unique<FPPCamera>(28.f, (float)WIDTH / HEIGHT, 0.1f, 600.f);		// Zoomed in secondary camera

	// Create a First-Person Camera Controller and attach a First-Person Perspective camera
	m_camera_controller = make_unique<FPCController>(m_input.get());
	m_camera_controller->set_camera(m_cam.get());
	m_camera_controller->set_secondary_camera(m_cam_zoom.get());

	// Set main camera for rendering
	gfx::rend->set_camera(m_camera_controller->get_active_camera());

	/*
		Declare UI for changing Scenes (Combo box)
	*/
	
	// Renderer
	m_model_renderer = new ModelRenderer(gfx::rend);
	m_sponza = m_model_renderer->load_model("models/sponza/sponza.obj");
	m_nanosuit = m_model_renderer->load_model("models/nanosuit/nanosuit.obj");

}

Application::~Application()
{
	delete m_model_renderer;

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
		// Begin frame
		perf::profiler->frame_start();
		Timer frame_time;

		// Block if paused
		while (m_paused);

		m_win->pump_messages();

		// Begin taking input
		m_input->begin();

		// Update CPU states
		update(dt);

		// Render GPU
		gfx::rend->begin();
		gfx::rend->set_camera(m_camera_controller->get_active_camera());

		// Submit models
		/*
			Renderers->Begin();

			Scene->Traverse()
				for each node:
					mod_rend->submit(..)
					part_rend->submit(..)
					etc..

			Renderers->End();
		*/
		m_model_renderer->begin();

		m_model_renderer->submit(m_sponza, DirectX::SimpleMath::Matrix::CreateScale(0.07f));
		for (int i = 0; i < 10; ++i)
		{
			// Try turning off shadows for certain submissions
			if (i % 2 == 0)
			{
				ModelRenderSpec spec;
				spec.casts_shadow = true;

				m_model_renderer->submit(m_nanosuit, DirectX::SimpleMath::Matrix::CreateScale(1.0) *
					DirectX::SimpleMath::Matrix::CreateTranslation(-45.f + i * 8.f, 0.f, 10.f), spec);
				continue;
			}

			m_model_renderer->submit(m_nanosuit, DirectX::SimpleMath::Matrix::CreateScale(1.0) *
				DirectX::SimpleMath::Matrix::CreateTranslation(-45.f + i * 8.f, 0.f, 10.f));
		}


		m_model_renderer->end();
			
		gfx::rend->render();
		gfx::rend->end();
		
		// End taking input
		m_input->end();
		
		// Re-size if requested
		if (m_should_resize)
		{
			on_resize(m_resized_client_area.first, m_resized_client_area.second);
			m_should_resize = false;
		}

		// End frame
		dt = frame_time.elapsed(Timer::Unit::Seconds);
		perf::profiler->frame_end();
	}
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

