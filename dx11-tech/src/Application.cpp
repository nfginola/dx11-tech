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

	// shader creation
	auto shader = m_gfx->create_shader_program("vs.hlsl", "ps.hlsl");

	// texture creation
	auto tex = m_gfx->create_texture(
		TextureDesc::make_2d(CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_TYPELESS, 800, 600, 1, 0, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)), 
		ViewDesc()
		.set(CD3D11_SHADER_RESOURCE_VIEW_DESC(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM))		// e.g gbuffer read
		.set(CD3D11_RENDER_TARGET_VIEW_DESC(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM)));		// e.g gbuffer write

	/*
		
		ResolveTarget and FboDesc are tightly coupled.
			ResolveTargets purpose is to add extra optional optional parameters for automatic subresource resolution for the FBO.

		FboDesc()
		.set(0, tex_hdl, ResolveTarget(resolve_tex_hdl, dest_subres = 0, src_subres = 0, DXGI_FORMAT = unknown) = {})		// if MSAA
		.set(1, tex_hdl2, ResolveTarget(resolve_tex_hdl, dest_subres = 0, src_subres = 0, DXGI_FORMAT = unknown) = {})
		.set(2, tex_hdl3, ResolveTarget(resolve_tex_hdl, dest_subres = 0, src_subres = 0, DXGI_FORMAT = unknown) = {})

		Where the creation of an FBO will verify that 
			- They are in order (if 0, 1, 2..)
				- If gaps, assert false.
				- Why do we allow this? To be transparent about the mapping between CPU and HLSL (in HLSL, we select which Render Target to write to using SV_TARGET0/1/2/..)
			- Each texture has a render target view associated with it
			- If a resolve texture is given
				- Check that the original texture has multisampling ON to verify

			- Handle three cases: https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-resolvesubresource
				- We will only handle Prestructured and Typed since thats what we have
					- If UNKNOWN (default) --> Check that both textures have same format and apply that DXGI Format for subresource resolution
					- If else (other cases, implement later)

		// for MSAA (open interface, for manual)
		m_gfx->resolve_subresource(dest_hdl, dest_subres, src_hdl, src_subres, DXGI_FORMAT);

		Point is:
			When the FBO is rendered to, it will automatically resolve any MSAA textures.
			--> Granting us the ability to use the resolved texture without any explicit commands.

		=======================

		FboHandle fbo = m_gfx->create_fbo(...)

		m_gfx->bind_fbo(fbo);
		m_gfx->draw(...)

		or..

		// STRICTLY for functions which operate on the state machine

		RenderPass rp{ fbo, ... }

		m_gfx->begin_pass(rp);			--> binds fbo
		// perhaps mimic https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkRenderPassBeginInfo.html
		// add clear values, render RECT, framebuffer
		...
		bind pipeline
		...
		bind per frame resource
		...
		bind per material resource
		m_gfx->draw();
		...
		bind per material resource
		m_gfx->draw();
		...
		m_gfx->end_pass();				--> resolve fbo, unbind RTVs

	
	*/

	/*
		PSO, mimic Vulkan
		Samplers, set by yourself
	
	
	*/


	/*
		rethink using binding tables in GfxApi.
		Maybe do as WickedEngine and return Texture, Shader and other objects which have internal state
			--> GPUResource : friend class GfxApi
			{
				std::shared_ptr<DXResource> internal_state;
			}

		use friend classes for intentional strong coupling to access the underlying DXBuffer/DXTexture
	
	
	*/

	// test copy
	auto tex2 = m_gfx->create_texture(tex,
		ViewDesc()
		.set(CD3D11_SHADER_RESOURCE_VIEW_DESC(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM)));

	// test using underlying
	auto tex3 = m_gfx->create_texture(tex,
		ViewDesc()
		.set(CD3D11_SHADER_RESOURCE_VIEW_DESC(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM)), true);


	std::cout << "haha\n";
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
