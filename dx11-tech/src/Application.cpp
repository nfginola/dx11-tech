#include "pch.h"
#include "Application.h"
#include "Window.h"

#include "Input.h"
#include "Timer.h"

#include "Graphics/GfxApi.h"

Application::Application()
{
	// Window and Input
	auto win_proc = [this](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT { return this->custom_win_proc(hwnd, uMsg, wParam, lParam); };
	m_win = make_unique<Window>(GetModuleHandle(nullptr), win_proc, 1920, 1080);
	m_input = make_unique<Input>(m_win->get_hwnd());
	m_gfx = make_unique<GfxApi>(make_unique<DXDevice>(m_win->get_hwnd(), m_win->get_client_width(), m_win->get_client_height()));

	GPUBuffer b;
	m_gfx->create_buffer(BufferDesc::constant(128), &b);


	GPUTexture t1;
	GPUTexture t2;
	m_gfx->create_texture(CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_UNORM, 1920, 1080), &t1);
	m_gfx->create_texture(CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1920, 1080, 1, 0, D3D11_BIND_RENDER_TARGET), &t2);

	GPUTexture* active_texture = &t1;
	/*
		Above gives clear possibility to easily branch and set active bindings in a flexible manner.
		same goes for the other bind functions! (e.g Pipeline)

		for example, we can switch to a Read-Through Pipeline with multiple viewports for Light Pass on a deferred renderer 
	*/

	//m_gfx->bind_resource(0, ShaderStage::eVertex, GPUAccess::eRead, active_texture);


	GPUTexture ds_24_8, ds_32_8, d_32;
	m_gfx->create_texture(TextureDesc::depth_stencil(DepthFormat::eD32, 1920, 1080, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE), &d_32);
	m_gfx->create_texture(TextureDesc::depth_stencil(DepthFormat::eD32_S8, 1920, 1080, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE), &ds_32_8);
	m_gfx->create_texture(TextureDesc::depth_stencil(DepthFormat::eD24_S8, 1920, 1080, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE), &ds_24_8);
	/*
		We can have different RenderPasses with different Depth Stencil View to be able to see the effects of less/more depth at runtime trivially!
	*/


	Framebuffer fb;
	FramebufferDesc fb_d({ t2 }, ds_32_8);
	m_gfx->create_framebuffer(fb_d, &fb);

	Framebuffer* active_fb = &fb;

	std::vector<D3D11_VIEWPORT> viewports = {
			CD3D11_VIEWPORT(0.f, 0.f, 1920.f, 1080.f),
			CD3D11_VIEWPORT(0.f, 0.f, 1920.f, 1080.f),
			CD3D11_VIEWPORT(0.f, 0.f, 1920.f, 1080.f),
	};

	/*
		For hardcoded textures and stuffs for Rendering Techniques, we can freely stick resources as private member objects in a Renderer or something.
		For resources that are to be handed out, they should be dynamically allocated, example:
			(shared_ptr<GPUBuffer> vertex_buffer = make_shared(..)
			(shared_ptr<GPUTexture> albedo_tex = make_shared(..)
			
			We can just allocate on heap immediately and not think about an allocator for now..
			We want this so that any higher level abstraction only hold on to a pointer. (GPUBuffer/GPUTexture internal is quite large, see below for size)
	*/
	std::cout << "sizeof GPUTexture: " << sizeof(GPUTexture) << "\n";
	std::cout << "sizeof GPUBuffer: " << sizeof(GPUBuffer) << "\n";

	Shader s1, s2;
	InputLayoutDesc d = InputLayoutDesc::get_layout<Vertex_POS_UV_NORMAL>();
	//auto p_d = PipelineDesc()
	//	.set_shaders(VertexShader(s1), PixelShader(s2))
	//	.set_input_layout(d);


	m_gfx->begin_pass(active_fb, DepthStencilClear::d1_s0());
}

Application::~Application()
{

}

void Application::run()
{
	while (m_win->is_alive() && m_app_alive)
	{
		while (m_paused);

		Timer frame_timer;
		m_win->pump_messages();
		//dx::get()->start_frame();
		m_input->begin();

		if (m_input->lmb_down())
		{
			std::cout << m_input->get_mouse_position().first << ", " << m_input->get_mouse_position().second << std::endl;
			//std::cout << m_input->get_mouse_dt().first << ", " << m_input->get_mouse_dt().second << std::endl;
		}
		if (m_input->key_pressed(Keys::R))
		{
			//dx::get()->hot_reload_shader({ 35 });
		}
		
		//dx::get()->clear_backbuffer(DirectX::Colors::MediumSeaGreen);
		//dx::get()->present(false);
		//dx::get()->end_frame();

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
