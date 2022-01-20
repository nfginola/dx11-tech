#include "pch.h"
#include "Application.h"
#include "Window.h"

#include "Input.h"
#include "Timer.h"


Application::Application()
{
	// Window and Input
	auto win_proc = [this](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT { return this->custom_win_proc(hwnd, uMsg, wParam, lParam); };
	m_win = make_unique<Window>(GetModuleHandle(nullptr), win_proc, 1920, 1080);
	m_input = make_unique<Input>(m_win->get_hwnd());
	
	// Initialize graphics device (singleton)
	GfxDevice::initialize(make_unique<DXDevice>(m_win->get_hwnd(), m_win->get_client_width(), m_win->get_client_height()));
	auto dev = GfxDevice::get();

	std::cout << "sizeof GPUTexture: " << sizeof(GPUTexture) << "\n";
	std::cout << "sizeof GPUBuffer: " << sizeof(GPUBuffer) << "\n";
	std::cout << "sizeof GfxApi: " << sizeof(GfxDevice) << "\n";

	// create depth tex
	GPUTexture d_32;
	dev->create_texture(TextureDesc::depth_stencil(DepthFormat::eD32, 1920, 1080, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE), &d_32);

	// create framebuffer for backbuffer
	dev->create_framebuffer(FramebufferDesc({ dev->get_backbuffer() }, d_32), &fb);

	// compile shaders
	ShaderBytecode vs_bc, ps_bc;
	dev->compile_shader(ShaderStage::eVertex, "VertexShader.hlsl", &vs_bc);
	dev->compile_shader(ShaderStage::ePixel, "PixelShader.hlsl", &ps_bc);
	
	// create shaders
	Shader vs, ps;
	dev->create_shader(ShaderStage::eVertex, vs_bc, &vs);
	dev->create_shader(ShaderStage::ePixel, ps_bc, &ps);

	// create pipeline
	auto p_d = PipelineDesc()
		.set_shaders(VertexShader(vs), PixelShader(ps))
		.set_input_layout(InputLayoutDesc::get_layout<Vertex_POS_UV_NORMAL>());
	dev->create_pipeline(p_d, &p);

	//// create render tex
	//GPUTexture render_tex;
	//dev->create_texture(CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_UNORM, 1920, 1080, 1, 0, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET), &render_tex);

	//// create framebuffer for render tex
	//dev->create_framebuffer(FramebufferDesc({ render_tex }), &r_fb);






	//dev->bind_viewports(viewports);
	//dev->begin_pass(active_fb, DepthStencilClear::d1_s0());
}

Application::~Application()
{
	GfxDevice::shutdown();
}

void Application::run()
{
	auto dev = GfxDevice::get();

	while (m_win->is_alive() && m_app_alive)
	{
		while (m_paused);

		Timer frame_timer;
		m_win->pump_messages();
		m_input->begin();

		if (m_input->lmb_down())
		{
			std::cout << m_input->get_mouse_position().first << ", " << m_input->get_mouse_position().second << std::endl;
			//std::cout << m_input->get_mouse_dt().first << ", " << m_input->get_mouse_dt().second << std::endl;
		}
		if (m_input->key_pressed(Keys::Left))
		{
			viewports[0].Width -= 40;
		}
		if (m_input->key_pressed(Keys::Right))
		{
			viewports[0].Width += 40;
		}
		if (m_input->key_pressed(Keys::Up))
		{
			viewports[0].Height -= 40;
		}
		if (m_input->key_pressed(Keys::Down))
		{
			viewports[0].Height += 40;
		}

		dev->frame_start();


		dev->begin_pass(&fb);
		dev->bind_viewports(viewports);
		dev->bind_pipeline(&p);
		dev->draw();
		dev->end_pass();



		dev->present();
		dev->frame_end();

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
