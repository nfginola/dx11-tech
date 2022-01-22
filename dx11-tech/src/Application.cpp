#include "pch.h"
#include "Application.h"
#include "Window.h"

#include "Input.h"
#include "Timer.h"

Application::Application()
{
	// Window render area dimension
	constexpr UINT WIN_WIDTH = 1920;
	constexpr UINT WIN_HEIGHT = 1080;

	// Resolution
	constexpr UINT WIDTH = 1280;
	constexpr UINT HEIGHT = 720;

	// Window and Input
	auto win_proc = [this](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT { return this->custom_win_proc(hwnd, uMsg, wParam, lParam); };
	m_win = make_unique<Window>(GetModuleHandle(nullptr), win_proc, WIN_WIDTH, WIN_HEIGHT);
	m_input = make_unique<Input>(m_win->get_hwnd());
	
	// Initialize graphics device (singleton)
	GfxDevice::initialize(make_unique<DXDevice>(m_win->get_hwnd(), WIDTH, HEIGHT));
	auto dev = GfxDevice::get();

	viewports = { CD3D11_VIEWPORT(0.f, 0.f, WIDTH, HEIGHT) };
		

	std::cout << "Framebuffer size: " << sizeof(Framebuffer) << "\n";
	std::cout << "GraphicsPipeline size: " << sizeof(GraphicsPipeline) << "\n";
	std::cout << "GPUBuffer size: " << sizeof(GPUBuffer) << "\n";
	std::cout << "GPUTexture size: " << sizeof(GPUTexture) << "\n";

	/*
		
		To-do: 
			- Add raster UAVs for OMSetRenderTargetsAndUAV			DONE
			- Add VB/IB binding to API								DONE
			- Draw triangle with VB/IB								DONE (Non-interleaved data too!)
			- Add HDR rendering and tone mapping					DONE (ACES tonemapping added)
			- Enable multisampling									DONE 
			- Add Resource Naming and Command Naming (11.4?)		
			- Add GPU query (maybe Set/EndEventMarker? 11.3)
			- Add Pipeline cache

			- Add hardcoded instance buffer on slot 31 and automatically have it bound if turned on for a Pass

	*/


	// setup geometry pass 
	{



		// MSAA
		{
			UINT sample_count = 4;
			UINT sample_quality = 8;

			// Create multisampled render target
			dev->create_texture(TextureDesc::make_2d(DXGI_FORMAT_R8G8B8A8_UNORM, WIDTH, HEIGHT, D3D11_BIND_RENDER_TARGET,
				1, 1, D3D11_USAGE_DEFAULT, 0, sample_count, sample_quality, 0), &r_tex_ms);	

			// Create multisampled depth texture with same count (specs requirement)
			dev->create_texture(TextureDesc::depth_stencil(DepthFormat::eD32, WIDTH, HEIGHT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,
				1, sample_count, sample_quality), &d_32);

			// Declare render target as MS target and declare backbuffer as MS resolve target
			dev->create_framebuffer(FramebufferDesc(
				{ &r_tex_ms }, &d_32,
				{ dev->get_backbuffer() }),
				&r_fb);
		}

		// Render directly
		{
			//// Create normal depth texture
			//dev->create_texture(TextureDesc::depth_stencil(DepthFormat::eD32, WIDTH, HEIGHT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE), &d_32);
			
			//// Render directly to backbuffer
			//dev->create_framebuffer(FramebufferDesc(
			//	{ dev->get_backbuffer() }, &d_32),
			//	&r_fb);
		}
		
		/*
			To turn on, uncomment code below and uncomment the final quad pass in main loop
		*/
		// Render to Texture
		{
			//// create depth tex
			//dev->create_texture(TextureDesc::depth_stencil(DepthFormat::eD32, WIDTH, HEIGHT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE), &d_32);

			//// create render to texture (HDR)
			//dev->create_texture(TextureDesc::make_2d(DXGI_FORMAT_R16G16B16A16_FLOAT, WIDTH, HEIGHT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET), &r_tex);

			//dev->create_framebuffer(FramebufferDesc(
			//	{ &r_tex }, &d_32),
			//	&r_fb);
		}

		// compile and create shaders
		Shader vs, ps;
		dev->compile_and_create_shader(ShaderStage::eVertex, "VertexShader.hlsl", &vs);
		dev->compile_and_create_shader(ShaderStage::ePixel, "PixelShader.hlsl", &ps);

		// Use VB/IB for triangle (non-interleaved)
		std::vector<DirectX::XMFLOAT3> positions = { { -0.5f, -0.5f, 0.f }, { 0.f, 0.5f, 0.f }, { 0.5f, -0.5f, 0.f } };
		std::vector<DirectX::XMFLOAT2> uvs = { {0.f, 1.f}, {1.f, 0.f}, {1.f, 1.f} };
		std::vector<DirectX::XMFLOAT3> normals = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f } };
		std::vector<uint32_t> indices = { 0, 1, 2 };
		dev->create_buffer(BufferDesc::vertex(positions.size() * sizeof(positions[0])), &vb_pos, SubresourceData(positions.data()));
		dev->create_buffer(BufferDesc::vertex(uvs.size() * sizeof(uvs[0])), &vb_uv, SubresourceData(uvs.data()));
		dev->create_buffer(BufferDesc::vertex(normals.size() * sizeof(normals[0])), &vb_nor, SubresourceData(normals.data()));
		dev->create_buffer(BufferDesc::index(indices.size() * sizeof(indices[0])), &ib, SubresourceData(indices.data()));

		// Interleaved layout
		auto layout = InputLayoutDesc()
			.append("POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0)
			.append("UV", DXGI_FORMAT_R32G32_FLOAT, 1)
			.append("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 3);

		// create pipeline
		auto p_d = PipelineDesc()
			.set_shaders(VertexShader(vs), PixelShader(ps))
			.set_input_layout(layout);
		dev->create_pipeline(p_d, &p);
	}


	// fullscreen quad pass to backbuffer
	{
		// create framebuffer for render to tex
		dev->create_framebuffer(FramebufferDesc({ dev->get_backbuffer() }), &fb);

		// create fullscreen quad shaders
		Shader fs_vs, fs_ps;
		dev->compile_and_create_shader(ShaderStage::eVertex, "fullscreenQuadVS.hlsl", &fs_vs);
		dev->compile_and_create_shader(ShaderStage::ePixel, "fullscreenQuadPS.hlsl", &fs_ps);

		// fullscreen quad pipeline
		auto rp_d = PipelineDesc()
			.set_shaders(VertexShader(fs_vs), PixelShader(fs_ps));
		dev->create_pipeline(rp_d, &r_p);

		// create and bind persistent sampler
		dev->create_sampler(SamplerDesc(), &def_samp);
		dev->bind_sampler(0, ShaderStage::ePixel, &def_samp);
	}
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
		




		// take input 
		{
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
		}

		// gpu frame start
		dev->frame_start();
	
		// geometry pass
		{
			dev->bind_viewports(viewports);

			dev->begin_pass(&r_fb);

			// draw
			dev->bind_pipeline(&p);
			GPUBuffer vbs[] = { vb_pos, vb_uv, GPUBuffer(), vb_nor };
			UINT strides[] = { sizeof(DirectX::XMFLOAT3), sizeof(DirectX::XMFLOAT2), 0, sizeof(DirectX::XMFLOAT3) };
			dev->bind_vertex_buffers(0, _countof(vbs), vbs, strides);
			// UINT instance_buffers = { world_mat_vib.Get() };
			// UINT instance_buf_strides = { sizeof(XMFLOAT4x4) };
			//dev->bind_vertex_buffers(gfxconstants::MAX_INPUT_SLOTS - _countof_instance_buffers, _countof(instance_buffers), &instance_buffers, &instance_buf_strides);
			dev->bind_index_buffer(&ib);
			dev->draw_indexed(3);

			dev->end_pass();
		}

		//// draw fullscreen pass
		//{
		//	dev->bind_resource(0, ShaderStage::ePixel, &r_tex);
		//	dev->bind_viewports(viewports);

		//	dev->begin_pass(&fb);
		//	dev->bind_pipeline(&r_p);
		//	dev->draw(6);
		//	dev->end_pass();
		//}






		m_input->end();

		// gpu frame end (does it help placing it at the end?)
		// when vsync is on, presents waits for vertical blank (hence it is blocking)
		// we can utilize that time between the block and vertical blank by placing Present at the end.
		dev->present();
		dev->frame_end();

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
