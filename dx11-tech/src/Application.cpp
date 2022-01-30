#include "pch.h"
#include "Application.h"
#include "Window.h"
#include "Input.h"
#include "Timer.h"
#include "Graphics/ImGuiDevice.h"

#include "FPCController.h"
#include "FPPCamera.h"

// Important that Globals is defined last, as the extern members need to be defined!
// We can define GfxGlobals.h if we want to have a separation layer later 
// Maybe we would like to have like WickedEngine where we have a Renderer.cpp and Renderer.hpp with all static functions
// And only allow GfxGlobals in that Renderer.cpp
// Just an idea.
#include "Globals.h"

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

	// Create perspective camera
	m_cam = make_unique<FPPCamera>(80.f, (float)WIDTH/HEIGHT);
	m_cam->set_position(0.f, 0.f, -5.f);
	
	// Create secondary camera
	m_cam2 = make_unique<FPPCamera>(80.f, (float)WIDTH / HEIGHT);
	m_cam2->set_position(0.f, 0.f, -15.f);

	// Create a First-Person Camera Controller and attach a First-Person Perspective camera
	m_camera_controller = make_unique<FPCController>(m_input.get());
	m_camera_controller->set_camera(m_cam.get());

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

			- Set Backbuffer to a Dockable Render Target?			TO-DO

			- Use ImGUI bar to show Frame Statistics				TO-DO
				- Bars for Full Frame
				- Numbers in ms on the side for parts

			- Add a Model class										TO-DO
				- Simply has Meshes and Materials (1:1 mapping)
				- Later down the line, we want to reformat for
					instancing.

			- Add a Simple Entity which holds a World Matrix		TO-DO
				- Holds a pointer to an existing Model (Flyweight)	

			- Bind Persistent Samplers (on the last slots stages)	TO-DO
				- Check MJP samples and DXTK for Common Samplers
				- Remember that shadows use diff. samplers

			- Check out JSON format and see if it is useful			TO-DO
				- Check out nlohmanns JSON parser

			- Add Pipeline cache									TO-DO
				- Check prev_pipeline
				- Also, use prev_ps to determine whether or not to
				  bind the new VS AND Input Layout!
				  They are connected!
					- Even though we duplicate Input Layouts
					  We can avoid binding the same ones
					  at least.

			- Try recreating view-space positions from only depth!
	*/

	viewports = { CD3D11_VIEWPORT(0.f, 0.f, WIDTH, HEIGHT) };

	// depth pre-pass
	{
		// try using depth-prepass to help with overdraw
		// but do this after youve loaded in models and have Pipeline Statistics so we can compare the invocations!
	}

	// setup geometry pass 
	{
		// Render to Texture
		{
			// create depth tex
			gfx::dev->create_texture(TextureDesc::depth_stencil(DepthFormat::eD32, WIDTH, HEIGHT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE), &d_32);

			// create render to texture (HDR)
			gfx::dev->create_texture(TextureDesc::make_2d(DXGI_FORMAT_R16G16B16A16_FLOAT, WIDTH, HEIGHT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET), &r_tex);

			gfx::dev->create_framebuffer(FramebufferDesc(
				{ &r_tex }, &d_32),
				&r_fb);
		}
		
		// make cbuffer
		gfx::dev->create_buffer(BufferDesc::constant(sizeof(PerFrameData)), &m_cb_per_frame);

		gfx::dev->create_buffer(BufferDesc::constant(512), &m_big_cb);	// x2 256


		// compile and create shaders
		Shader vs, ps;
		gfx::dev->compile_and_create_shader(ShaderStage::eVertex, "VertexShader.hlsl", &vs);
		gfx::dev->compile_and_create_shader(ShaderStage::ePixel, "PixelShader.hlsl", &ps);

		// Use VB/IB for triangle (non-interleaved)
		std::vector<DirectX::XMFLOAT3> positions = { { -0.5f, -0.5f, 0.f }, { 0.f, 0.5f, 0.f }, { 0.5f, -0.5f, 0.f } };
		std::vector<DirectX::XMFLOAT2> uvs = { {0.f, 1.f}, {1.f, 0.f}, {1.f, 1.f} };
		std::vector<DirectX::XMFLOAT3> normals = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f } };
		std::vector<uint32_t> indices = { 0, 1, 2 };
		gfx::dev->create_buffer(BufferDesc::vertex(positions.size() * sizeof(positions[0])), &vb_pos, SubresourceData(positions.data()));
		gfx::dev->create_buffer(BufferDesc::vertex(uvs.size() * sizeof(uvs[0])), &vb_uv, SubresourceData(uvs.data()));
		gfx::dev->create_buffer(BufferDesc::vertex(normals.size() * sizeof(normals[0])), &vb_nor, SubresourceData(normals.data()));
		gfx::dev->create_buffer(BufferDesc::index(indices.size() * sizeof(indices[0])), &ib, SubresourceData(indices.data()));

		// Interleaved layout
		auto layout = InputLayoutDesc()
			.append("POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0)
			.append("UV", DXGI_FORMAT_R32G32_FLOAT, 1)
			.append("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 2);

		// create pipeline
		auto p_d = PipelineDesc()
			.set_shaders(VertexShader(vs), PixelShader(ps))
			.set_input_layout(layout);
		gfx::dev->create_pipeline(p_d, &p);
	}

	// fullscreen quad pass to backbuffer
	{
		// create framebuffer for render to tex
		gfx::dev->create_framebuffer(FramebufferDesc({ gfx::dev->get_backbuffer() }), &fb);

		// create fullscreen quad shaders
		Shader fs_vs, fs_ps;
		gfx::dev->compile_and_create_shader(ShaderStage::eVertex, "fullscreenQuadVS.hlsl", &fs_vs);
		gfx::dev->compile_and_create_shader(ShaderStage::ePixel, "fullscreenQuadPS.hlsl", &fs_ps);

		// fullscreen quad pipeline
		auto rp_d = PipelineDesc()
			.set_shaders(VertexShader(fs_vs), PixelShader(fs_ps));
		gfx::dev->create_pipeline(rp_d, &r_p);

		// create and bind persistent sampler
		gfx::dev->create_sampler(SamplerDesc(), &def_samp);
		gfx::dev->bind_sampler(0, ShaderStage::ePixel, &def_samp);
	}
}

Application::~Application()
{
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
		
		// Check window message pumping overhead
		{
			auto _ = FrameProfiler::ScopedCPU("WM Pump");
			m_win->pump_messages();
		}
		// Break as soon as possible
		if (!m_app_alive || !m_win->is_alive())
			break;

		m_input->begin();
			
		// Update CPU states
		update(dt);

		// Update persistent per frame data
		m_cb_dat.view_mat = m_camera_controller->get_active_camera()->get_view_mat();
		m_cb_dat.proj_mat = m_camera_controller->get_active_camera()->get_proj_mat();

		// Update per draw
		// https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0
		/*
			Maybe its not such a good idea to do this now for draw calls (premature optimization).
			Lets just stick with the normal cbuffers for now.

			Each CBElement is aligned(256)
		*/
		m_cb_elements[0].world_mat = DirectX::XMMatrixTranslation(2.f, 0.f, 0.f);
		m_cb_elements[1].world_mat = DirectX::XMMatrixTranslation(-2.f, 0.f, 0.f);

		// Update graphics
		gfx::dev->frame_start();
		gfx::imgui->begin_frame();

		// Upload per frame data to GPU
		gfx::dev->map_copy(&m_cb_per_frame, SubresourceData(&m_cb_dat, sizeof(m_cb_dat)));

		// Bind per frame data
		gfx::dev->bind_constant_buffer(0, ShaderStage::eVertex, &m_cb_per_frame, 0);

		// Upload per draw data to GPU at once
		gfx::dev->map_copy(&m_big_cb, SubresourceData(m_cb_elements.data(), (UINT)m_cb_elements.size() * sizeof(m_cb_elements[0])));

		{
			auto _ = FrameProfiler::Scoped("Geometry Pass");
			gfx::dev->begin_pass(&r_fb);
			gfx::dev->bind_viewports(viewports);
			// draw
			gfx::dev->bind_pipeline(&p);
			GPUBuffer vbs[] = { vb_pos, vb_uv, vb_nor };
			UINT strides[] = { sizeof(DirectX::XMFLOAT3), sizeof(DirectX::XMFLOAT2), sizeof(DirectX::XMFLOAT3) };
			gfx::dev->bind_vertex_buffers(0, _countof(vbs), vbs, strides);
			gfx::dev->bind_index_buffer(&ib);
	
			gfx::annotator->begin_event("Draw Triangles");
			gfx::dev->bind_constant_buffer(1, ShaderStage::eVertex, &m_big_cb, 0);
			gfx::dev->draw_indexed(3);
			gfx::dev->bind_constant_buffer(1, ShaderStage::eVertex, &m_big_cb, 1);
			gfx::dev->draw_indexed(3);
			gfx::annotator->end_event();
			gfx::dev->end_pass();
		}

		{
			auto _ = FrameProfiler::Scoped("Fullscreen Pass");
			gfx::dev->begin_pass(&fb);
			gfx::dev->bind_resource(0, ShaderStage::ePixel, &r_tex);
			gfx::dev->bind_viewports(viewports);
			gfx::dev->bind_pipeline(&r_p);
			gfx::dev->draw(6);

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
			gfx::imgui->draw();		// Draw overlay

			gfx::dev->end_pass();
		}	

		// when vsync is on, presents waits for vertical blank (hence it is blocking)
		// we can utilize that time between the block and vertical blank by placing other miscellaneous end functions BEFORE present!
		m_input->end();

		// Check present block CPU block (if vsync)
		{
			auto _ = FrameProfiler::Scoped("Presentation");
			gfx::dev->present();
		}

		gfx::imgui->end_frame();
		gfx::dev->frame_end();

		
		++m_curr_frame;
		dt = frame_time.elapsed(Timer::Unit::Seconds);
		perf::profiler->frame_end();
	}




}

LRESULT Application::custom_win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/*
	
		We may want to consider having an Engine custom_win_proc which have defaults
		and the application win_proc can extend it. 
		Essentailly the same method as we are doing with the Window and Application
	*/

	if (gfx::imgui->win_proc(hwnd, uMsg, wParam, lParam))
		return true;

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

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Application::update(float dt)
{
	// Possess different cameras
	if (m_input->key_pressed(Keys::D1))		m_camera_controller->set_camera(m_cam.get());
	if (m_input->key_pressed(Keys::D2))		m_camera_controller->set_camera(m_cam2.get());

	// Shader reload test
	if (m_input->key_pressed(Keys::R))		gfx::dev->recompile_pipeline_shaders_by_name("fullscreenQuadPS");

	// Update camera controller
	m_camera_controller->update(dt);
}
