#include "pch.h"
#include "Graphics/Renderer/Renderer.h"
#include "Camera/Camera.h"
#include "Graphics/API/GfxDevice.h"
#include "Graphics/API/ImGuiDevice.h"
#include "Profiler/FrameProfiler.h"

#include "Graphics/CommandBucket/GfxCommand.h"


// Temp
#include "Graphics/Model.h"
#include "Graphics/ModelManager.h"

// Global dependencies
namespace gfx
{
	Renderer* rend = nullptr;
	extern GfxDevice* dev;
	extern ImGuiDevice* imgui;
	extern GPUAnnotator* annotator;

	extern ModelManager* model_mgr;	// Temp
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

Renderer::~Renderer()
{
}

Renderer::Renderer()
{
	assert(gfx::dev != nullptr);
	assert(perf::profiler != nullptr);

	// Declare UI 
	ImGuiDevice::add_ui("default ui", [&]() { declare_ui(); });
	ImGuiDevice::add_ui("profiler", [&]() { declare_profiler_ui();  });
	ImGuiDevice::add_ui("shader reloading", [&]() { declare_shader_reloader_ui();  });


	auto sc_dim = gfx::dev->get_sc_dim();
	viewports =
	{
		CD3D11_VIEWPORT(0.f, 0.f, (FLOAT)sc_dim.first, (FLOAT)sc_dim.second),	// To Swapchain (changes when swapchain is resized)
		CD3D11_VIEWPORT(0.f, 0.f, (FLOAT)sc_dim.first, (FLOAT)sc_dim.second)	// Render to texture (stays the same for Geometry Pass)
	};

	// temp	// Load models
	//m_models.push_back(gfx::model_mgr->load_model("models/sponza/sponza.obj", "Sponza"));
	//m_models.push_back(gfx::model_mgr->load_model("models/rungholt/rungholt.obj", "Rungholt"));
	//m_models.push_back(gfx::model_mgr->load_model("models/lost_empire/lost_empire.obj", "LostEmpire"));

	// setup geometry pass 
	{
		// Should take in some RendererInitDesc to initialize settings (e.g vsync and other misc.)
		create_resolution_dependent_resources(sc_dim.first, sc_dim.second);

		// make cbuffer
		m_cb_per_frame = gfx::dev->create_buffer(BufferDesc::constant(sizeof(PerFrameData)));
		m_big_cb = gfx::dev->create_buffer(BufferDesc::constant(256 * m_cb_elements.size()));
		//m_big_cb = gfx::dev->create_buffer(BufferDesc::constant(sizeof(CBElement)));


		// compile and create shaders
		ShaderHandle vs, ps;
		vs = gfx::dev->compile_and_create_shader(ShaderStage::eVertex, "VertexShader.hlsl");
		ps = gfx::dev->compile_and_create_shader(ShaderStage::ePixel, "PixelShader.hlsl");

		// Interleaved layout
		auto layout = InputLayoutDesc()
			.append("POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0)
			.append("UV", DXGI_FORMAT_R32G32_FLOAT, 1)
			.append("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 2);

		// create pipeline
		auto p_d = PipelineDesc()
			.set_shaders(VertexShader(vs), PixelShader(ps))
			.set_input_layout(layout);
		//gfx::dev->create_pipeline(p_d, &p);
		p = gfx::dev->create_pipeline(p_d);
	}

	// fullscreen quad pass to backbuffer
	{
		// create RenderPass for render to tex
		fb = gfx::dev->create_renderpass(RenderPassDesc({ gfx::dev->get_backbuffer(), }));

		// create fullscreen quad shaders
		ShaderHandle fs_vs, fs_ps;
		fs_vs = gfx::dev->compile_and_create_shader(ShaderStage::eVertex, "fullscreenQuadVS.hlsl");
		fs_ps = gfx::dev->compile_and_create_shader(ShaderStage::ePixel, "fullscreenQuadPS.hlsl");

		// fullscreen quad pipeline
		auto rp_d = PipelineDesc()
			.set_shaders(VertexShader(fs_vs), PixelShader(fs_ps));
		r_p = gfx::dev->create_pipeline(rp_d);

		// create and bind persistent sampler
		def_samp = gfx::dev->create_sampler(SamplerDesc());
		gfx::dev->bind_sampler(0, ShaderStage::ePixel, def_samp);
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
	//begin();

	/*
		forget these for now	
		ParticleRenderer(Renderer core_renderer)
		TerrainRenderer(Renderer core_renderer)

		=========================

		ModelRenderer(Renderer core_renderer)
		ModelRenderer
			void submit_model(Model* model, KeyProperties props, world_mat)		--> KeyProperties defined in core_renderer
			{
				// create DrawItem(s) (e.model --> grab geom, materials) ??????
				// or DrawItem(s) have already been prepared and we simply grab them

				// create Key(s) from properties from Entity ??????

				
				for each (key, draw_item) in draw_items:
					if (model->transparent)
						core_renderer->submit_transparent(key, draw_item)
					else
						core_renderer->submit_opaque(key, draw_item)
			}	
	
	
	*/

	// Update persistent per frame camera data
	m_cb_dat.view_mat = m_main_cam->get_view_mat();
	m_cb_dat.proj_mat = m_main_cam->get_proj_mat();

	// Upload per frame data to GPU
	gfx::dev->map_copy(m_cb_per_frame, SubresourceData(&m_cb_dat, sizeof(m_cb_dat)));

	// Bind per frame data (should be bound to like slot 14 as reserved space)
	gfx::dev->bind_constant_buffer(0, ShaderStage::eVertex, m_cb_per_frame, 0);

	// Geometry Pass
	{
		auto _ = FrameProfiler::Scoped("Geometry Pass");

		gfx::dev->begin_pass(r_fb);
		gfx::dev->bind_viewports({ viewports[1] });
		
		///*
		//	A model renderer should be responsible for this
		//*/
		//const auto& model = m_models[0];
		//const auto& meshes = model->get_meshes();
		//const auto& materials = model->get_materials();

		//// Update world matrix
		//m_cb_elements[0].world_mat = DirectX::SimpleMath::Matrix::CreateScale(0.07);
		//m_cb_elements[1].world_mat = DirectX::SimpleMath::Matrix::CreateScale(0.07) * DirectX::SimpleMath::Matrix::CreateTranslation(0.f, 0.f, 300.f);
		//m_cb_elements[2].world_mat = DirectX::SimpleMath::Matrix::CreateScale(0.07) * DirectX::SimpleMath::Matrix::CreateTranslation(0.f, 0.f, -300.f);

		///*
		//	Copy scene matrices to giant GPU cbuffer
		//	This is per mesh, as all submeshes to that mesh should have the same matrix.
		//	We simply bind the correct range for each draw call (each submesh has the range of its parent mesh)
		//*/
		//auto big_copy = m_copy_bucket.add_command<gfxcommand::CopyToBuffer>(0, 0);
		//big_copy->buffer = m_big_cb;
		//big_copy->data = m_cb_elements.data();
		//big_copy->data_size = m_cb_elements.size() * sizeof(m_cb_elements[0]);

		//for (int inst = 0; inst < 1; ++inst)
		//{
		//	for (int i = 0; i < meshes.size(); ++i)
		//	{
		//		const auto& mesh = meshes[i];
		//		const auto& mat = materials[i];

		//		uint8_t vbs_in = model->get_vb().size();
		//		uint8_t cbs_in = 1;	// Per Object CBuffer
		//		uint8_t textures_in = 1;
		//		uint8_t samplers_in = 0;

		//		// Key based on texture
		//		uint64_t key = mat->get_texture(Material::Texture::eAlbedo).hdl;

		//		// Create draw call
		//		size_t aux_size = gfxcommand::aux::bindtable::get_size(vbs_in, cbs_in, textures_in, samplers_in);
		//		auto cmd = m_main_bucket.add_command<gfxcommand::Draw>(key, aux_size);
		//		cmd->ib = model->get_ib();
		//		cmd->index_count = mesh.index_count;
		//		cmd->index_start = mesh.index_start;
		//		cmd->vertex_start = mesh.vertex_start;
		//		cmd->pipeline = p;			// Should come from material

		//		// Fill draw call payload (bind table)
		//		auto payload = gfxcommand::aux::bindtable::Filler(gfxcommandpacket::get_aux_memory(cmd), vbs_in, cbs_in, samplers_in, textures_in);
		//		for (int i = 0; i < vbs_in; ++i)
		//			payload.add_vb(std::get<0>(model->get_vb()[i]).hdl, std::get<1>(model->get_vb()[i]), std::get<2>(model->get_vb()[i]));

		//		payload.add_cb(m_big_cb.hdl, (uint8_t)ShaderStage::eVertex, 1, inst);
		//		payload.add_texture(mat->get_texture(Material::Texture::eAlbedo).hdl, (uint8_t)ShaderStage::ePixel, 0);
		//		payload.validate();
		//	}
		//}

		// No need to sort copy bucket
		m_copy_bucket.flush();

		m_opaque_bucket.sort();
		m_opaque_bucket.flush();

		//m_main_bucket.sort();
		//m_main_bucket.flush();
	
		gfx::dev->end_pass();
	}

	// Fullscreen Pass
	{
		auto _ = FrameProfiler::Scoped("Fullscreen Pass");
		gfx::dev->begin_pass(fb);
		gfx::dev->bind_resource(0, ShaderStage::ePixel, r_tex);
		gfx::dev->bind_viewports(viewports);
		gfx::dev->bind_pipeline(r_p);
		gfx::dev->draw(6);

		gfx::imgui->draw();		// Draw ImGUI overlay 

		gfx::dev->end_pass();
	}





	//end();
}

void Renderer::create_resolution_dependent_resources(UINT width, UINT height)
{
	// Render to Texture
	{
		// viewport
		viewports[1].Width = (FLOAT)width;
		viewports[1].Height = (FLOAT)height;

		// create depth tex
		d_32 = gfx::dev->create_texture(TextureDesc::depth_stencil(DepthFormat::eD32, width, height, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE));

		// create render to texture (HDR)
		r_tex = gfx::dev->create_texture(TextureDesc::make_2d(DXGI_FORMAT_R16G16B16A16_FLOAT, width, height, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET));

		r_fb = gfx::dev->create_renderpass(RenderPassDesc(
			{ r_tex }, d_32));
	}
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

