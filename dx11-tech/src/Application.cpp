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

#include "Entity.h"

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
	DiskTextureManager::initialize(gfx::dev);
	MaterialManager::initialize(gfx::tex_mgr);
	ModelManager::initialize(gfx::dev, gfx::mat_mgr);

	// Declare UI 
	gfx::imgui->add_ui_callback("default ui", [&]() { declare_ui(); });
	gfx::imgui->add_ui_callback("profiler", [&]() { declare_profiler_ui();  });
	gfx::imgui->add_ui_callback("shader reloading", [&]() { declare_shader_reloader_ui();  });

	// Create perspective camera
	m_cam = make_unique<FPPCamera>(90.f, (float)WIDTH/HEIGHT, 0.1f, 1000.f);
	m_cam->set_position(0.f, 0.f, -5.f);
	
	// Create a First-Person Camera Controller and attach a First-Person Perspective camera
	m_camera_controller = make_unique<FPCController>(m_input.get());
	m_camera_controller->set_camera(m_cam.get());

	// Load models
	m_models.push_back(gfx::model_mgr->load_model("models/sponza/sponza.fbx", "Sponza"));

	// Test mock entity
	/*
		Replace with enTT for flexibility
	*/
	Entity e;
	e.add_component<eModelComponent>(new ModelComponent(m_models[0]));
	e.get_component<eTransformComponent>().set_position(5.f, 0.f, 0.f);

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

			- Create a Pipeline Manager								TO-DO
				- Create Pipeline (naming too)
				- Retrieve Pipeline

			- Add enTT library for flexibility						TO-DO

			- Add a Simple Entity which holds a World Matrix		TO-DO
				- Holds a pointer to an existing Model (Flyweight)	

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

			- Bind Persistent Samplers (on the last slots stages)	TO-DO
				- Check MJP samples and DXTK for Common Samplers
				- Remember that shadows use diff. samplers

			- Create Swap chain class?								TO-DO
				- Refactoring work

			- Grab Pipeline Query									TO-DO
				- Refactoring work

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

			- Try recreating view-space positions from only depth!	TO-DO
	*/

	viewports = 
	{ 
		CD3D11_VIEWPORT(0.f, 0.f, WIDTH, HEIGHT),	// To Swapchain (changes when swapchain is resized)
		CD3D11_VIEWPORT(0.f, 0.f, WIDTH, HEIGHT)	// Render to texture (stays the same for Geometry Pass)
	};

	// depth pre-pass
	{
		// try using depth-prepass to help with overdraw
		// but do this after youve loaded in models and have Pipeline Statistics so we can compare the invocations!
	}

	// setup geometry pass 
	{
		create_resolution_dependent_resources(WIDTH, HEIGHT);
		
		// make cbuffer
		gfx::dev->create_buffer(BufferDesc::constant(sizeof(PerFrameData)), &m_cb_per_frame);
		gfx::dev->create_buffer(BufferDesc::constant(256 * 3), &m_big_cb);	

		// compile and create shaders
		Shader vs, ps;
		gfx::dev->compile_and_create_shader(ShaderStage::eVertex, "VertexShader.hlsl", &vs);
		gfx::dev->compile_and_create_shader(ShaderStage::ePixel, "PixelShader.hlsl", &ps);

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

	// sponza requires wrapping texture, we will also use anisotropic filtering here (16) 
	D3D11_SAMPLER_DESC repeat{};
	repeat.Filter = D3D11_FILTER_ANISOTROPIC;
	repeat.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	repeat.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	repeat.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	repeat.MinLOD = -FLT_MAX;
	repeat.MaxLOD = FLT_MAX;
	repeat.MipLODBias = 0.0f;
	repeat.MaxAnisotropy = 16;
	repeat.ComparisonFunc = D3D11_COMPARISON_NEVER;
	gfx::dev->create_sampler(repeat, &repeat_samp);
	gfx::dev->bind_sampler(1, ShaderStage::ePixel, &repeat_samp);


}

Application::~Application()
{
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

		// Update graphics
		gfx::dev->frame_start();
		gfx::imgui->begin_frame();

		// Upload per frame data to GPU
		gfx::dev->map_copy(&m_cb_per_frame, SubresourceData(&m_cb_dat, sizeof(m_cb_dat)));

		// Bind per frame data
		gfx::dev->bind_constant_buffer(0, ShaderStage::eVertex, &m_cb_per_frame, 0);

		// Upload per draw data to GPU at once
		gfx::dev->map_copy(&m_big_cb, SubresourceData(m_cb_elements.data(), (UINT)m_cb_elements.size() * sizeof(m_cb_elements[0])));
	
		// Geometry Pass
		{
			auto _ = FrameProfiler::Scoped("Geometry Pass");
			gfx::dev->begin_pass(&r_fb);
			gfx::dev->bind_viewports({ viewports[1] });
			gfx::dev->bind_constant_buffer(1, ShaderStage::eVertex, &m_big_cb, 0);

			// draw models
			gfx::annotator->begin_event("Draw Models");

			gfx::dev->bind_pipeline(&p);
			for (const auto& model : m_models)
			{
				gfx::dev->bind_vertex_buffers(0, (UINT)model->get_vbs().size(), model->get_vbs().data(), model->get_vb_strides().data());
				gfx::dev->bind_index_buffer(model->get_ib());

				// draw each submesh
				const auto& meshes = model->get_meshes();
				const auto& materials = model->get_materials();
				for (int i = 0; i < meshes.size(); ++i)
				{
					const auto& mesh = meshes[i];
					const auto& mat = materials[i];
					gfx::dev->bind_resource(0, ShaderStage::ePixel, mat->get_texture(Material::Texture::eAlbedo));
					gfx::dev->draw_indexed(mesh.index_count, mesh.index_start, mesh.vertex_start);
				}
			}

			gfx::annotator->end_event();
			gfx::dev->end_pass();
		}

		// Fullscreen Pass
		{
			auto _ = FrameProfiler::Scoped("Fullscreen Pass");
			gfx::dev->begin_pass(&fb);
			gfx::dev->bind_resource(0, ShaderStage::ePixel, &r_tex);
			gfx::dev->bind_viewports(viewports);
			gfx::dev->bind_pipeline(&r_p);
			gfx::dev->draw(6);

			gfx::imgui->draw();		// Draw ImGUI overlay 

			gfx::dev->end_pass();
		}	

		// when vsync is on, presents waits for vertical blank (hence it is blocking)
		// we can utilize that time between the block and vertical blank by placing other miscellaneous end functions BEFORE present!
		m_input->end();

		// Check present block CPU block (if vsync)
		{
			auto _ = FrameProfiler::Scoped("Presentation");
			gfx::dev->present(m_vsync);
		}

		gfx::imgui->end_frame();
		gfx::dev->frame_end();

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
		if (item_current_idx == 0)		create_resolution_dependent_resources(2560, 1440);
		if (item_current_idx == 1)		create_resolution_dependent_resources(1920, 1080);
		if (item_current_idx == 2)		create_resolution_dependent_resources(1280, 720);
		if (item_current_idx == 3)		create_resolution_dependent_resources(640, 360);
		if (item_current_idx == 4)		create_resolution_dependent_resources(384, 216);
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

void Application::create_resolution_dependent_resources(UINT width, UINT height)
{
	// Render to Texture
	{
		// viewport
		viewports[1].Width = (FLOAT)width;
		viewports[1].Height = (FLOAT)height;

		// create depth tex
		gfx::dev->create_texture(TextureDesc::depth_stencil(DepthFormat::eD32, width, height, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE), &d_32);

		// create render to texture (HDR)
		gfx::dev->create_texture(TextureDesc::make_2d(DXGI_FORMAT_R16G16B16A16_FLOAT, width, height, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET), &r_tex);

		gfx::dev->create_framebuffer(FramebufferDesc(
			{ &r_tex }, &d_32),
			&r_fb);
	}
}

void Application::on_resize(UINT width, UINT height)
{
	if (width == m_win->get_client_width() && height == m_win->get_client_height())
		return;

	fmt::print("resize with dimensions: [Width: {}], [Height: {}]\n", m_resized_client_area.first, m_resized_client_area.second);

	m_win->resize_client(width, height);

	gfx::dev->resize_swapchain(width, height);

	// recreate framebuffer for backbuffer
	gfx::dev->create_framebuffer(FramebufferDesc({ gfx::dev->get_backbuffer() }), &fb);

	// resize vp for backbuffer
	viewports[0].Width = (FLOAT)width;
	viewports[0].Height = (FLOAT)height;
	viewports[0].TopLeftX = 0.f;
	viewports[0].TopLeftY = 0.f;
}

void Application::update(float dt)
{
	// Update camera controller
	m_camera_controller->update(dt);

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
	m_cb_elements[0].world_mat = DirectX::XMMatrixScaling(0.07f, 0.07f, 0.07f);
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

