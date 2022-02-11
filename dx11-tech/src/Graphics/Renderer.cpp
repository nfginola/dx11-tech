#include "pch.h"
#include "Graphics/Renderer.h"
#include "Graphics/GfxDevice.h"
#include "Profiler/FrameProfiler.h"
#include "Graphics/ImGuiDevice.h"
#include "Camera/Camera.h"
#include "Graphics/Drawable/ICustomDrawable.h"


// Temp
#include "Graphics/Model.h"
#include "Graphics/ModelManager.h"

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


Renderer::Renderer()
{
	assert(gfx::dev != nullptr);
	assert(perf::profiler != nullptr);

	auto sc_dim = gfx::dev->get_sc_dim();


	viewports =
	{
		CD3D11_VIEWPORT(0.f, 0.f, (FLOAT)sc_dim.first, (FLOAT)sc_dim.second),	// To Swapchain (changes when swapchain is resized)
		CD3D11_VIEWPORT(0.f, 0.f, (FLOAT)sc_dim.first, (FLOAT)sc_dim.second)	// Render to texture (stays the same for Geometry Pass)
	};

	// temp	// Load models
	m_models.push_back(gfx::model_mgr->load_model("models/sponza/sponza.fbx", "Sponza"));

	// setup geometry pass 
	{
		// Should take in some RendererInitDesc to initialize settings (e.g vsync and other misc.)
		create_resolution_dependent_resources(sc_dim.first, sc_dim.second);

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

void Renderer::create_resolution_dependent_resources(UINT width, UINT height)
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

Renderer::~Renderer()
{
}

//void Renderer::submit(ICustomDrawable* drawable)
//{
//	m_custom_drawables.push_back(drawable);
//}

void Renderer::set_camera(Camera* cam)
{
	m_main_cam = cam;
}

void Renderer::render()
{
	// Update persistent per frame data
	m_cb_dat.view_mat = m_main_cam->get_view_mat();
	m_cb_dat.proj_mat = m_main_cam->get_proj_mat();

	// Update per draw
	// https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0
	/*
		Maybe its not such a good idea to do this now for draw calls (premature optimization).
		Lets just stick with the normal cbuffers for now.

		Each CBElement is aligned(256)
	*/
	m_cb_elements[0].world_mat = DirectX::XMMatrixScaling(0.07f, 0.07f, 0.07f);


	//for (auto& dbl : m_custom_drawables)
	//	dbl->pre_render();

	//for (auto& dbl : m_custom_drawables)
	//	dbl->on_render();

	
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

	// Check present block CPU block (if vsync)
	{
		auto _ = FrameProfiler::Scoped("Presentation");
		gfx::dev->present(m_vsync);
	}

	gfx::imgui->end_frame();
	gfx::dev->frame_end();

}

void Renderer::on_resize(UINT width, UINT height)
{
	create_resolution_dependent_resources(width, height);
}

