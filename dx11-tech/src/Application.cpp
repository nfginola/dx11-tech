#include "pch.h"
#include "Application.h"
#include "Window.h"

#include "Input.h"
#include "Timer.h"

// Testing
#include "Graphics/GfxCommon.h"
#include "Graphics/DXTexture.h"
#include "Graphics/DXShader.h"

Application::Application()
{
	// Window and Input
	auto win_proc = [this](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT { return this->custom_win_proc(hwnd, uMsg, wParam, lParam); };
	m_win = make_unique<Window>(GetModuleHandle(nullptr), win_proc, 1920, 1080);
	m_input = make_unique<Input>(m_win->get_hwnd());
	m_dx_device = make_unique<DXDevice>(m_win->get_hwnd(), m_win->get_client_width(), m_win->get_client_height());

	/*
	
	ezdx(make_unique dx device)
	...


	TEXTURE CREATION
	=======

	TextureHandle my_tex = ezdx->create_texture(TextureDesc::from(
	));
	ezdx->create_srv(my_tex, [](ID3D11Texture2D* tex) { return CD3D11(...); });

	TextureHandle tex_copy = ezdx->create_texture(my_tex);						// create from handle --> same underlying Texture2D but no views!
	ezdx->create_srv(tex_copy, [](ID3D11Texture2D* tex) { return CD3D11(...); }	// same tex2d, but different view!
	========

	TEXTURE BINDING
	========
	ezdx->bind_texture(Slot, Access, Stage, textureHandle);
		-- if bind read --> check if internal texture is already bound as write, if yes, unbind.
			-- if prev == readWrite --> unbind UAV for slot at stage
			-- if prev == write		--> unbind RTV fully
	========

	CONSTANT BUFFER (EXPLICIT)
	========

	ezdx->bind_constant_buffer(slot, stage, bufferHandle)
		-- internally checks the DXBuffer if it is of type Constant Buffer
	========

	OTHER BUFFER (VIEWS)
	=======
	ezdx->bind_buffer(slot, access, stage, bufferHandle)
		-- internally checks that access, bufferhandle and views all match and exist
	=======

	FRAMEBUFFERS (COLLECTION OF TEXTURE TO RENDER TO)
	=======
	FboHandle ezdx->create_fbo( [ { tex1, settings1 }, { tex2, settings2 } ] )
	
	ezdx->bind_fbo(fboHandle);

	ezdx->clear_fbo(fboHandle, { depth_stencil, [ tex1clear, tex2clear, ... ] });

	=======

	MISCELLANEOUS (FOR LATER) OPTIMIZATIONS
	=======
	ezdx->clear_constant_buffers()		// unbinds all bound cbuffers so that update latency is not slow! (it is dependent on how many places the res is bound)
	=======


	
	*/

	

	// til next time: Work on creating DXBuffers 
	DXTexture my_tex(m_dx_device.get(), TextureDesc::make_2d(CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_UNORM, 800, 600)));
	my_tex.create_srv_ext(m_dx_device.get(), [](ID3D11Texture2D* tex) { return CD3D11_SHADER_RESOURCE_VIEW_DESC(tex, D3D11_SRV_DIMENSION_TEXTURE2D); });

	// Example mistakes, which are guarded against!
	//my_tex.create_srv_ext(m_dx_device.get(), [](ID3D11Texture2D* tex) { return CD3D11_SHADER_RESOURCE_VIEW_DESC(tex, D3D11_SRV_DIMENSION_TEXTURE1D); });
	//my_tex.create_srv_ext(m_dx_device.get(), [](ID3D11Texture1D* tex) { return CD3D11_SHADER_RESOURCE_VIEW_DESC(tex, D3D11_SRV_DIMENSION_TEXTURE2D); });


	//my_tex.create_srv(m_dx_device.get(), D3D11_SHADER_RESOURCE_VIEW_DESC{});
	//my_tex.create_srv_ext(m_dx_device.get(), [](ID3D11Texture1D* tex) { return CD3D11_SHADER_RESOURCE_VIEW_DESC(tex, D3D11_SRV_DIMENSION_TEXTURE1D); });
	
	DXShader shader(m_dx_device.get(), "a.hlsl", "b.hlsl");
}

Application::~Application()
{
//	dx::shutdown();
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
