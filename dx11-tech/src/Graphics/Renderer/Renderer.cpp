#include "pch.h"
#include "Graphics/Renderer/Renderer.h"
#include "Graphics/API/GfxDevice.h"
#include "Graphics/API/ImGuiDevice.h"
#include "Graphics/CommandBucket/GfxCommand.h"
#include "Profiler/FrameProfiler.h"
#include "Camera/Camera.h"



// Global dependencies
namespace gfx
{
	Renderer* rend = nullptr;
	extern GfxDevice* dev;
	extern ImGuiDevice* imgui;
	extern GPUAnnotator* annotator;
}

namespace perf
{
	extern FrameProfiler* profiler;
}

void Renderer::initialize()
{
	if (!gfx::rend)
		gfx::rend = new Renderer();
}

void Renderer::shutdown()
{
	if (gfx::rend)
	{
		delete gfx::rend;
		gfx::rend = nullptr;
	}
}

void Renderer::set_camera(Camera* cam)
{
	m_main_cam = cam;
}

Renderer::Renderer()
{
	assert(gfx::dev != nullptr);
	assert(perf::profiler != nullptr);

	// Declare UI 
	ImGuiDevice::add_ui("default ui", [&]() { declare_ui(); });
	ImGuiDevice::add_ui("profiler", [&]() { declare_profiler_ui();  });
	ImGuiDevice::add_ui("shader reloading", [&]() { declare_shader_reloader_ui();  });

	m_cb_per_frame = gfx::dev->create_buffer(BufferDesc::constant(sizeof(PerFrameData)));


	auto sc_dim = gfx::dev->get_sc_dim();
	viewports =
	{
		CD3D11_VIEWPORT(0.f, 0.f, (FLOAT)sc_dim.first, (FLOAT)sc_dim.second),	// To Swapchain (changes when swapchain is resized)
		CD3D11_VIEWPORT(0.f, 0.f, (FLOAT)sc_dim.first, (FLOAT)sc_dim.second)	// Render to texture (stays the same for Geometry Pass)
	};

	// setup geometry pass 
	// Should take in some RendererInitDesc to initialize settings (e.g vsync and other misc.)
	create_resolution_dependent_resources(sc_dim.first, sc_dim.second);

	// create and bind persistent sampler
	def_samp = gfx::dev->create_sampler(SamplerDesc());
	gfx::dev->bind_sampler(0, ShaderStage::ePixel, def_samp);
	
	// setup light pass
	{
		ShaderHandle fs_vs, fs_ps;
		fs_vs = gfx::dev->compile_and_create_shader(ShaderStage::eVertex, "fullscreenQuadVS.hlsl");
		fs_ps = gfx::dev->compile_and_create_shader(ShaderStage::ePixel, "fullscreenQuadPS.hlsl");
		
		m_lightpass_pipe = gfx::dev->create_pipeline(PipelineDesc()
			.set_shaders(VertexShader(fs_vs), PixelShader(fs_ps)));
	}

	// setup final post-proc pass
	// directly to backbuffer (may change in the future, e.g use Compute to fill the backbuffer via RTV)
	{
		m_backbuffer_out_rp = gfx::dev->create_renderpass(RenderPassDesc({ gfx::dev->get_backbuffer() }));

		ShaderHandle vs, ps;
		vs = gfx::dev->compile_and_create_shader(ShaderStage::eVertex, "finalQuadVS.hlsl");
		ps = gfx::dev->compile_and_create_shader(ShaderStage::ePixel, "finalQuadPS.hlsl");

		m_final_pipeline = gfx::dev->create_pipeline(PipelineDesc()
			.set_shaders(VertexShader(vs), PixelShader(ps)));
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

	repeat_samp = gfx::dev->create_sampler(repeat);
	gfx::dev->bind_sampler(1, ShaderStage::ePixel, repeat_samp);






}

void Renderer::begin()
{
	gfx::dev->frame_start();
	gfx::imgui->begin_frame();
}

void Renderer::end()
{
	// Present
	{
		// Profiler has to end before device frame end
		auto _ = FrameProfiler::Scoped("Presentation");
		gfx::dev->present(m_vsync);
	}

	gfx::imgui->end_frame();
	gfx::dev->frame_end();
}

void Renderer::render()
{
	// Update persistent per frame camera data
	m_cb_dat.view_mat = m_main_cam->get_view_mat();
	m_cb_dat.proj_mat = m_main_cam->get_proj_mat();

	// Upload per frame data to GPU
	gfx::dev->map_copy(m_cb_per_frame, SubresourceData(&m_cb_dat, sizeof(m_cb_dat)));

	// Bind per frame data (should be bound to like slot 14 as reserved space)
	gfx::dev->bind_constant_buffer(0, ShaderStage::eVertex, m_cb_per_frame, 0);

	// No need to sort copy bucket
	m_copy_bucket.flush();

	// Geometry Pass
	{
		auto _ = FrameProfiler::Scoped("Geometry Pass");

		gfx::dev->begin_pass(m_gbuffer_res.rp);
		gfx::dev->bind_viewports({ viewports[1], viewports[1], viewports[1] });

		m_opaque_bucket.sort();
		m_opaque_bucket.flush();

		gfx::dev->end_pass();
	}

	// Lightpass (Fullscreen)
	{
		auto _ = FrameProfiler::Scoped("Light Pass");

		gfx::dev->begin_pass(m_lightpass_rp);
		gfx::dev->bind_viewports({ viewports[1] });
		gfx::dev->bind_pipeline(m_lightpass_pipe);
	
		m_gbuffer_res.read_bind(gfx::dev);
		
		gfx::dev->draw(6);

		gfx::dev->end_pass();
	}

	// Write to backbuffer for final post-proc (tone mapping and gamma correction)
	{
		auto _ = FrameProfiler::Scoped("Final Fullscreen Pass");
		gfx::dev->begin_pass(m_backbuffer_out_rp);
		gfx::dev->bind_viewports(viewports);
		gfx::dev->bind_pipeline(m_final_pipeline);
		gfx::dev->bind_resource(0, ShaderStage::ePixel, m_lightpass_output);
		gfx::dev->draw(6);

		// Draw ImGUI overlay on top of everything
		gfx::imgui->draw();

		gfx::dev->end_pass();
	}
}

void Renderer::create_resolution_dependent_resources(UINT width, UINT height)
{
	if (m_allocated)
	{
		gfx::dev->free_texture(d_32);
		gfx::dev->free_texture(m_lightpass_output);
		m_gbuffer_res.free(gfx::dev);
		m_allocated = false;
	}

	// Render to Texture
	{
		// viewport
		viewports[1].Width = (FLOAT)width;
		viewports[1].Height = (FLOAT)height;

		// create 32-bit depth tex
		d_32 = gfx::dev->create_texture(TextureDesc::depth_stencil(DepthFormat::eD32, width, height, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE));

		// create gbuffer textures and pass
		m_gbuffer_res.create_gbuffers(gfx::dev, width, height);
		m_gbuffer_res.create_rp(gfx::dev, d_32);

		// create light pass output (16-bit per channel HDR)
		m_lightpass_output = gfx::dev->create_texture(TextureDesc::make_2d(DXGI_FORMAT_R16G16B16A16_FLOAT, width, height, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET));
		m_lightpass_rp = gfx::dev->create_renderpass(RenderPassDesc({ m_lightpass_output }));	// no depth
	}

	m_allocated = true;
}

void Renderer::on_resize(UINT width, UINT height)
{
	viewports[0].Width = (FLOAT)width;
	viewports[0].Height = (FLOAT)height;
}

void Renderer::on_change_resolution(UINT width, UINT height)
{
	create_resolution_dependent_resources(width, height);
}

void Renderer::declare_ui()
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

	//ImGui::Checkbox("Proto", &m_proto);


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
	static int item_current_idx = 1; // Here we store our selection data as an index.
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
		if (item_current_idx == 0)		on_change_resolution(2560, 1440);
		if (item_current_idx == 1)		on_change_resolution(1920, 1080);
		if (item_current_idx == 2)		on_change_resolution(1280, 720);
		if (item_current_idx == 3)		on_change_resolution(640, 360);
		if (item_current_idx == 4)		on_change_resolution(384, 216);
	}

	ImGui::End();

}

void Renderer::declare_profiler_ui()
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

void Renderer::declare_shader_reloader_ui()
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

void Renderer::GBuffer::create_gbuffers(GfxDevice* dev, UINT width, UINT height)
{
	// create GBuffers
	albedo = gfx::dev->create_texture(TextureDesc::make_2d(DXGI_FORMAT_R8G8B8A8_UNORM, width, height,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
		1, 1));
	normal = gfx::dev->create_texture(TextureDesc::make_2d(DXGI_FORMAT_R8G8B8A8_UNORM, width, height,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
		1, 1));
	world = gfx::dev->create_texture(TextureDesc::make_2d(DXGI_FORMAT_R8G8B8A8_UNORM, width, height,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
		1, 1));
}

void Renderer::GBuffer::create_rp(GfxDevice* dev, TextureHandle depth)
{
	rp = gfx::dev->create_renderpass(RenderPassDesc(
		{
			albedo,
			normal,
			world
		}, depth));
}

void Renderer::GBuffer::read_bind(GfxDevice* dev)
{
	dev->bind_resource(0, ShaderStage::ePixel, albedo);
	dev->bind_resource(1, ShaderStage::ePixel, normal);
	dev->bind_resource(2, ShaderStage::ePixel, world);
}

void Renderer::GBuffer::free(GfxDevice* dev)
{
	gfx::dev->free_texture(albedo);
	gfx::dev->free_texture(normal);
	gfx::dev->free_texture(world);
}
