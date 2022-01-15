#include "pch.h"
#include "Application.h"
#include "Window.h"

#include "Graphics/dx.h"
#include "Input.h"
#include "Timer.h"

Application::Application()
{
	// Window and Input
	auto win_proc = [this](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT { return this->custom_win_proc(hwnd, uMsg, wParam, lParam); };
	m_win = make_unique<Window>(GetModuleHandle(nullptr), win_proc, 1920, 1080);
	m_input = make_unique<Input>(m_win->get_hwnd());

	// DX API
	dx::init(make_unique<DXDevice>(m_win->get_hwnd(), m_win->get_client_width(), m_win->get_client_height()));

	/*
	
		We want at some point in the future to encapsulate this into an Engine.
		The Engine would consist of:
			- Window
			- Input
			- SceneManager
				- Scene (Application provides)
					- Composition based Entities

			- Renderer (Provides App with higher level constructs)
					- dx (Graphics API backend)
				- Camera
				- Mesh
				- Material
				- Shader
				- Light
			- Physics (Provides App with higher level constructs)
					- BulletPhysics (Physics API backend)
				- Colllision boxes
				- Physics simulation (e.g gravity)

		Application is required to do (every frame):
			engine->start_frame();
			scene->run();
			engine->end_frame();
	
	
	*/

	auto vb = dx::get()->create_vertex_buffer();
	auto ib = dx::get()->create_index_buffer();
	auto b = dx::get()->create_buffer();
	auto tex = dx::get()->create_texture();
	auto shader = dx::get()->create_shader("a.hlsl", "a.hlsl");
	auto p = dx::get()->create_pipeline();

	dx::get()->bind_vertex_buffer(vb);
	dx::get()->bind_index_buffer(ib);
	dx::get()->upload_to_buffer((void*)m_win.get(), 512, b);

	dx::get()->bind_buffer(0, BAccess::eConstant, ShaderStage::eHull, b);
	dx::get()->bind_buffer(1, BAccess::eConstant, ShaderStage::eGeometry, b);		// we somehow need to handle UAV bound to some other stage --> binding as SRV problem
	dx::get()->bind_buffer(2, BAccess::eConstant, ShaderStage::ePixel, b);			// solve this problem when we implement something with UAV
	dx::get()->bind_texture(0, TAccess::eRead, ShaderStage::ePixel, tex);

	dx::get()->bind_pipeline(p);

	// should not be a public function: ShaderHandle is passed to a PipelineDescriptor
	//dx::get()->bind_shader(shader);

	dx::get()->draw_fullscreen_quad();


	auto b_desc = BufferDesc::make_constant(32);
	auto t_desc = TextureDesc::make_2d(nullptr, 0, 1920, 1080, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE);
	auto p_desc = PipelineDesc::make(shader);

	// Viewport and Scissors will be kept Dynamic

	std::cout << "init\n";

}

Application::~Application()
{
	dx::shutdown();
}

void Application::run()
{
	while (m_win->is_alive() && m_app_alive)
	{
		while (m_paused);

		Timer frame_timer;
		m_win->pump_messages();
		dx::get()->start_frame();
		m_input->begin();

		if (m_input->lmb_down())
		{
			std::cout << m_input->get_mouse_position().first << ", " << m_input->get_mouse_position().second << std::endl;
			//std::cout << m_input->get_mouse_dt().first << ", " << m_input->get_mouse_dt().second << std::endl;
		}
		if (m_input->key_pressed(Keys::R))
		{
			dx::get()->hot_reload_shader({ 35 });
		}
		
		dx::get()->clear_backbuffer(DirectX::Colors::MediumSeaGreen);
		dx::get()->present(false);
		dx::get()->end_frame();

		m_input->end();

		auto sec_elapsed = frame_timer.elapsed();
		//std::cout << "fps: " << 1.f / sec_elapsed << std::endl;
	}
}

LRESULT Application::custom_win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/*
	
		We may want to consider having an Engine custom_win_proc which have defaults
		and the application win_proc can extend it. 
		Essentailly the same method as we are doing with the Window and Application
	
	*/

	//if (m_appIsAlive && m_engine)
	//{
	//	auto imGuiFunc = m_engine->GetImGuiHook();
	//	if (imGuiFunc)
	//	{
	//		imGuiFunc(hwnd, uMsg, wParam, lParam);
	//	}
	//}

	switch (uMsg)
	{
		// DirectXTK Mouse and Keyboard (Input)
	case WM_ACTIVATEAPP:
	{
		//if (m_engine && m_engine->GetInput())
		//{
		//	m_engine->GetInput()->process_keyboard(uMsg, wParam, lParam);
		//	m_engine->GetInput()->process_mouse(uMsg, wParam, lParam);
		//}
		//break;

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
		//if (m_engine && m_engine->GetInput())
		//{
		//	m_engine->GetInput()->process_mouse(uMsg, wParam, lParam);
		//}
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
		//if (m_engine && m_engine->GetInput())
		//{
		//	m_engine->GetInput()->process_keyboard(uMsg, wParam, lParam);
		//}
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
		// NOTE: We may want to turn off so that we can scale freely in both dimensions.
		// Because we would like to keep the aspect ratio of the initial resolution! To not handle it, lets just turn off free scaling and resize only on Fullscreen enter/exit

		// We want to hook this to ImGui viewport later
		//if (m_resizeCallback)
		//	m_resizeCallback(LOWORD(lParam), HIWORD(lParam));
		//std::cout << "should resize\n";
		// dont resize here (a lot of calls)
		break;
	}
	case WM_ENTERSIZEMOVE:
	{
		std::cout << "should pause to prep for resize\n";
		m_paused = true;
		break;
	}
	case WM_EXITSIZEMOVE:
	{
		std::cout << "should resize\n";
		m_paused = false;
		break;
	}

	// Input message
	case WM_SYSKEYDOWN:
	{
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{
			// Custom Alt + Enter to toggle windowed borderless
			// m_win->set_fullscreen(!m_win->is_fullscreen());
			// Resizing will be handled through WM_SIZE through a subsequent WM
			std::cout << "should resize\n";
		}
		break;
	}

	default:
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	}

	return 0;
}
