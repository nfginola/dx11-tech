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
		CD3D11_VIEWPORT(0.f, 0.f, (FLOAT)sc_dim.first, (FLOAT)sc_dim.second),	// Render to texture (stays the same for Geometry Pass)
		CD3D11_VIEWPORT(0.f, 0.f, (FLOAT)m_shadow_map_resolution, (FLOAT)m_shadow_map_resolution)								// Shadow (temp hardcoded)
	};

	// setup geometry pass 
	// Should take in some RendererInitDesc to initialize settings (e.g vsync and other misc.)
	create_resolution_dependent_resources(sc_dim.first, sc_dim.second);

	// setup depth only pass
	{
		// Using 16-bit because we are using Orthographic (linearly distributed values, so 16 bit is enough)
		/*
			https://gamedev.net/forums/topic/692064-shadow-mapping/5355978/
			eD16 if need performance
		*/
		m_dir_d32 = gfx::dev->create_texture(TextureDesc::depth_stencil(
			DepthFormat::eD32, (UINT)m_shadow_map_resolution, (UINT)m_shadow_map_resolution,
			D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE));
		m_dir_rp = gfx::dev->create_renderpass(RenderPassDesc({}, m_dir_d32));
	}

	// setup light pass
	{
		ShaderHandle fs_vs, fs_ps;
		fs_vs = gfx::dev->compile_and_create_shader(ShaderStage::eVertex, "lightPassVS.hlsl");
		fs_ps = gfx::dev->compile_and_create_shader(ShaderStage::ePixel, "lightPassPS.hlsl");

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

		m_final_pipe = gfx::dev->create_pipeline(PipelineDesc()
			.set_shaders(VertexShader(vs), PixelShader(ps)));
	}
	
	// commonly used samplers
	{
		// point
		def_samp = gfx::dev->create_sampler(SamplerDesc());
		gfx::dev->bind_sampler(0, ShaderStage::ePixel, def_samp);

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

	
	// shadow related data
	{
		// create shadow sampler
		D3D11_SAMPLER_DESC ss_desc{};
		ss_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		ss_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		ss_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		ss_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		ss_desc.BorderColor[0] = 1.f;
		ss_desc.BorderColor[1] = 1.f;
		ss_desc.BorderColor[2] = 1.f;
		ss_desc.BorderColor[3] = 1.f;
		ss_desc.MinLOD = -FLT_MAX;
		ss_desc.MaxLOD = FLT_MAX;
		ss_desc.MipLODBias = 0.0f;
		ss_desc.MaxAnisotropy = 1;
		ss_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		m_shadow_sampler = gfx::dev->create_sampler(ss_desc);

		gfx::dev->bind_sampler(3, ShaderStage::ePixel, m_shadow_sampler);

		m_per_light_cb = gfx::dev->create_buffer(BufferDesc::constant(1 * sizeof(PerLightData)));



	}




	// depth only pipe
	{
		auto vs_depth = gfx::dev->compile_and_create_shader(ShaderStage::eVertex, "depthOnlyVS.hlsl");
		auto ps_depth = gfx::dev->compile_and_create_shader(ShaderStage::ePixel, "depthOnlyPS.hlsl");
		auto do_layout = InputLayoutDesc().append("POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0);

		m_shared_resources.depth_only_pipe = gfx::dev->create_pipeline(PipelineDesc()
			.set_shaders(VertexShader(vs_depth), PixelShader(ps_depth))
			.set_input_layout(do_layout)
			.set_rasterizer(RasterizerDesc::no_backface_cull()));
	}

	// deferred gpass pipe
	{
		ShaderHandle vs, ps;
		vs = gfx::dev->compile_and_create_shader(ShaderStage::eVertex, "gpassVS.hlsl");
		ps = gfx::dev->compile_and_create_shader(ShaderStage::ePixel, "gpassPS.hlsl");

		// interleaved layout
		auto layout = InputLayoutDesc()
			.append("POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0)
			.append("UV", DXGI_FORMAT_R32G32_FLOAT, 1)
			.append("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 2);

		// create pipeline
		auto p_d = PipelineDesc()
			.set_shaders(VertexShader(vs), PixelShader(ps))
			.set_input_layout(layout);

		m_shared_resources.deferred_gpass_pipe = gfx::dev->create_pipeline(p_d);
	}





	//setup_SDSM();
}

void Renderer::setup_SDSM()
{
	//// Parallel min/max reduction
	//{
	//	// Texture to block reduction
	//	ShaderHandle cs = gfx::dev->compile_and_create_shader(ShaderStage::eCompute, "SDSM_ReduceTexToBuffer.hlsl");
	//	m_compute_pipe = gfx::dev->create_compute_pipeline(ComputePipelineDesc(ComputeShader(cs)));

	//	// Block to block reduction
	//	ShaderHandle cs2 = gfx::dev->compile_and_create_shader(ShaderStage::eCompute, "SDSM_FinalReduction.hlsl");
	//	m_compute_pipe2 = gfx::dev->create_compute_pipeline(ComputePipelineDesc(ComputeShader(cs2)));

	//	// Compute splits
	//	ShaderHandle cs3 = gfx::dev->compile_and_create_shader(ShaderStage::eCompute, "SDSM_ComputeSplits.hlsl");
	//	m_compute_pipe3 = gfx::dev->create_compute_pipeline(ComputePipelineDesc(ComputeShader(cs3)));


	//	// 60 x 34 is the amount of blocks from Compute Test
	//	/*
	//		We can derive these numbers by
	//			ceil(RESOLUTION_WIDTH / 32)			Assumed to use 32x32x1 threads per block
	//			ceil(RESOLUTION_HEIGHT / 32)

	//		Further passes use the prev numbers multiplied and divide them by 1024
	//			e.g

	//			60 x 34 = 2040
	//			ceil(2040/1024) = 2

	//		keep doing until that number = 1 and then call it one last time.



	//	*/
	//	float* null_data = new float[60 * 34];

	//	// Reset maxes to 0
	//	std::memset(null_data, 0, 60 * 34 * sizeof(float));
	//	m_rw_buf = gfx::dev->create_buffer(BufferDesc::structured(sizeof(float), { 0, 60 * 34 }, D3D11_BIND_UNORDERED_ACCESS),
	//		SubresourceData(null_data, 60 * 34 * sizeof(float)));

	//	// Reset mins to 1
	//	std::memset(null_data, 1, 60 * 34 * sizeof(float));
	//	m_rw_buf2 = gfx::dev->create_buffer(BufferDesc::structured(sizeof(float), { 0, 60 * 34 }, D3D11_BIND_UNORDERED_ACCESS),
	//		SubresourceData(null_data, 60 * 34 * sizeof(float)));

	//	delete[] null_data;

	//	// Compute splits (4) from CS and use in light pass
	//	m_rw_splits = gfx::dev->create_buffer(BufferDesc::structured(sizeof(float), { 0, 4 }, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE));


	//	m_staging[0] = gfx::dev->create_buffer(BufferDesc::staging(2 * sizeof(float)));	// min/max
	//	m_staging[1] = gfx::dev->create_buffer(BufferDesc::staging(2 * sizeof(float)));	// min/max
	//	m_staging[2] = gfx::dev->create_buffer(BufferDesc::staging(2 * sizeof(float)));	// min/max
	//}


}

void Renderer::compute_SDSM()
{
	//// Parallel min/max reduction
	//{
	//	// Reduce to 60 x 34 (Texture to buffer)
	//	auto x_blocks = std::ceilf(m_curr_resolution.first / 32.f);
	//	const auto y_blocks = std::ceilf(m_curr_resolution.second / 32.f);
	//	{
	//		auto _ = FrameProfiler::Scoped("Reduction 1");
	//		gfx::dev->bind_resource(0, ShaderStage::eCompute, m_d_32);
	//		gfx::dev->bind_resource_rw(0, ShaderStage::eCompute, m_rw_buf);
	//		gfx::dev->bind_resource_rw(1, ShaderStage::eCompute, m_rw_buf2);
	//		gfx::dev->bind_compute_pipeline(m_compute_pipe);
	//		//gfx::dev->dispatch(60, 34, 1);

	//		gfx::dev->dispatch(x_blocks, y_blocks, 1);
	//	}


	//	// (Buffer to buffer)
	//	// Reduce to 2 (note that 60 x 34 = 2040, we cant do it in one block)
	//	{
	//		auto _ = FrameProfiler::Scoped("Reduction 2");
	//		gfx::dev->bind_resource_rw(0, ShaderStage::eCompute, m_rw_buf);
	//		gfx::dev->bind_resource_rw(1, ShaderStage::eCompute, m_rw_buf2);
	//		gfx::dev->bind_constant_buffer(GLOBAL_PER_FRAME_CB_SLOT, ShaderStage::eCompute, m_cb_per_frame);
	//		gfx::dev->bind_compute_pipeline(m_compute_pipe2);

	//		x_blocks = std::ceilf((x_blocks * y_blocks) / 1024.f);
	//		gfx::dev->dispatch(x_blocks, 1, 1);
	//	}

	//	// Reduce to 1
	//	{
	//		auto _ = FrameProfiler::Scoped("Reduction 3");
	//		gfx::dev->bind_resource_rw(0, ShaderStage::eCompute, m_rw_buf);
	//		gfx::dev->bind_resource_rw(1, ShaderStage::eCompute, m_rw_buf2);
	//		gfx::dev->bind_compute_pipeline(m_compute_pipe2);

	//		x_blocks = std::ceilf(x_blocks / 1024.f);
	//		gfx::dev->dispatch(x_blocks, 1, 1);
	//	}

	//	{
	//		auto _ = FrameProfiler::Scoped("Compute Split");
	//		gfx::dev->bind_resource_rw(0, ShaderStage::eCompute, m_rw_buf2);		// Note these are swapped (0 --> mins, 1 --> maxes)
	//		gfx::dev->bind_resource_rw(1, ShaderStage::eCompute, m_rw_buf);
	//		gfx::dev->bind_resource_rw(2, ShaderStage::eCompute, m_rw_splits);
	//		gfx::dev->bind_compute_pipeline(m_compute_pipe3);

	//		gfx::dev->dispatch(1, 1, 1);
	//	}

	//	// Delayed CPU read (buffered)
	//	{
	//		// Triple buffered to minimize sync stalls
	//		// Map will wait for the Copy to finish
	//		// https://stackoverflow.com/questions/40808759/id3d11devicecontextmap-slow-performance
	//		{
	//			auto _ = FrameProfiler::Scoped("Min/Max Copy");
	//			// fill 0 - sizeof(float) with first sizeof(float) in src
	//			gfx::dev->copy_resource_region(m_staging[(m_curr_frame + 2) % 3], CopyRegionDst(0, 0), m_rw_buf, CopyRegionSrc(0, { 0, 0, 0, sizeof(float), 1, 1 }));

	//			// fill sizeof(float) -> sizeof(float) * 2 with first sizeof(float) in src
	//			gfx::dev->copy_resource_region(m_staging[(m_curr_frame + 2) % 3], CopyRegionDst(0, sizeof(float)), m_rw_buf2, CopyRegionSrc(0, { 0, 0, 0, sizeof(float), 1, 1 }));
	//		}
	//		{
	//			auto _ = FrameProfiler::Scoped("Min/Max Read");
	//			gfx::dev->map_read_temp(m_staging[m_curr_frame % 3]);
	//		}
	//	}
	//}

}

void Renderer::create_resolution_dependent_resources(UINT width, UINT height)
{
	if (m_allocated)
	{
		gfx::dev->free_texture(m_d_32);
		gfx::dev->free_texture(m_lightpass_output);
		m_gbuffer_res.free(gfx::dev);
		m_allocated = false;
	}

	m_curr_resolution.first = width;
	m_curr_resolution.second = height;

	// Render to Texture
	{
		// viewport
		viewports[1].Width = (FLOAT)width;
		viewports[1].Height = (FLOAT)height;

		// create 32-bit depth tex
		m_d_32 = gfx::dev->create_texture(TextureDesc::depth_stencil(DepthFormat::eD32, width, height, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE));

		// create gbuffer textures and pass
		m_gbuffer_res.create_gbuffers(gfx::dev, width, height);
		m_gbuffer_res.create_rp(gfx::dev, m_d_32);

		// create light pass output (16-bit per channel HDR)
		m_lightpass_output = gfx::dev->create_texture(TextureDesc::make_2d(DXGI_FORMAT_R16G16B16A16_FLOAT, width, height, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET));
		m_lightpass_rp = gfx::dev->create_renderpass(RenderPassDesc({ m_lightpass_output }));	// no depth
	}

	m_allocated = true;
}





void Renderer::begin()
{
	gfx::dev->frame_start();
	gfx::imgui->begin_frame();

	++m_curr_frame;
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


float get_cascade_split(float lambda, UINT current_partition, UINT number_of_partitions, float near_z, float far_z)
{
	// https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-parallel-split-shadow-maps-programmable-gpus

	float exp = (float)current_partition / number_of_partitions;

	// Logarithmic split scheme
	float Ci_log = near_z * std::powf((far_z / near_z), exp);

	// Uniform split scheme
	float Ci_uni = near_z + (far_z - near_z) * exp;

	// Lambda [0, 1]
	float Ci = lambda * Ci_log + (1.f - lambda) * Ci_uni;
	return Ci;
}


void Renderer::render()
{
	// Update persistent per frame camera data
	m_cb_dat.view_mat = m_main_cam->get_view_mat();
	m_cb_dat.proj_mat = m_main_cam->get_proj_mat();
	m_cb_dat.inv_proj_mat = m_main_cam->get_proj_mat().Invert();

	// Get cascade splits
	{
		// Hardcoded directional light for now
		//auto light_direction = DirectX::SimpleMath::Vector3{ 0.2f, -0.8f, 0.2f };
		auto light_direction = m_sun_direction;
		light_direction.Normalize();


		float cam_near_z = 0.1f;
		float cam_far_z = 1000.f;

		float clip_range = cam_far_z - cam_near_z;
		
		// Get splits
		std::array<float, NUM_CASCADES> cascade_splits;
		for (int i = 0; i < cascade_splits.size(); ++i)
		{
			cascade_splits[i] = get_cascade_split(m_lambda, i + 1, (UINT)cascade_splits.size(), cam_near_z, cam_far_z) / clip_range;
		}
	
		// Calculate orthographic projection matrix for each cascade
		std::array<DirectX::SimpleMath::Matrix, cascade_splits.size()> matrices;
		float lastSplitDist = 0.0;
		for (uint32_t i = 0; i < cascade_splits.size(); i++)
		{
			float splitDist = cascade_splits[i];
		
			// Setup NDC frustum points
			std::array<DirectX::SimpleMath::Vector3, 8> frustum_points =
			{
				// Near plane
				DirectX::SimpleMath::Vector3(-1.0f,  1.0f, 0.0f),	// Top left
				DirectX::SimpleMath::Vector3(1.0f,  1.0f, 0.0f),	// Top right
				DirectX::SimpleMath::Vector3(1.0f, -1.0f, 0.0f),	// Bottom right
				DirectX::SimpleMath::Vector3(-1.0f, -1.0f, 0.0f),	// Bottom left

				// Far plane
				DirectX::SimpleMath::Vector3(-1.0f, 1.0f, 1.f),		// Top left
				DirectX::SimpleMath::Vector3(1.0f,  1.0f, 1.f),		// Top right
				DirectX::SimpleMath::Vector3(1.0f, -1.0f, 1.f),		// Bottom right
				DirectX::SimpleMath::Vector3(-1.0f, -1.0f, 1.f)		// Bottom left
			};
			
			// Transform frustum points to world space
			for (auto& p : frustum_points)
			{
#ifdef REVERSE_Z_DEPTH
				p.z = 1.f - p.z;
#endif	
				// .. and transform into world space ..
				auto clip_to_world = (m_main_cam->get_view_mat() * m_main_cam->get_proj_mat()).Invert();
				auto p_v4 = DirectX::SimpleMath::Vector4(p);
				p_v4.w = 1.f;

				auto p_v4_ws  = DirectX::SimpleMath::Vector4::Transform(p_v4, clip_to_world);
				// Homogenous to cartesian (reverse perspective division)
				p_v4_ws /= p_v4_ws.w;

				p = DirectX::SimpleMath::Vector3(p_v4_ws);
			}

			// Modify frustum so we have the subfrustum for this cascade
			for (uint32_t i = 0; i < 4; i++) 
			{
				auto dist = frustum_points[i + 4] - frustum_points[i];
				frustum_points[i + 4] = frustum_points[i] + (dist * splitDist);
				frustum_points[i] = frustum_points[i] + (dist * lastSplitDist);
			}

			// Get frustum center
			// Sum contribution of all points, and find average to find the center of the frustum
			auto frustum_center = DirectX::SimpleMath::Vector3(0.f);
			for (const auto& p : frustum_points)
				frustum_center += p;
			frustum_center /= (float)frustum_points.size();
			
			// Get max spherical radius for the frustum
			// Notice that our new space (origin) is at frustum center.
			// Thus, we find the min/max relative to the frustum center (sphere space)
			float radius = 0.f;
			for (const auto& p : frustum_points)
			{
				float distance = (p - frustum_center).Length();
				radius = std::max(radius, distance);
			}
			radius = std::ceilf(radius * 16.f) / 16.f;		// ??


			auto max_extents = DirectX::SimpleMath::Vector3(radius);
			auto min_extents = -max_extents;

			DirectX::SimpleMath::Matrix light_view = DirectX::XMMatrixLookAtLH(frustum_center - light_direction * -min_extents.z, frustum_center, { 0.0f, 1.0f, 0.0f });

#ifdef REVERSE_Z_DEPTH
			DirectX::SimpleMath::Matrix ortho_mat = DirectX::XMMatrixOrthographicOffCenterLH(
				min_extents.x, max_extents.x, min_extents.y, max_extents.y, max_extents.z - min_extents.z, 0.f);
#else
			DirectX::SimpleMath::Matrix ortho_mat = DirectX::XMMatrixOrthographicOffCenterLH(
				min_extents.x, max_extents.x, min_extents.y, max_extents.y, 0.f, max_extents.z - min_extents.z);
#endif

			// Avoid shadow shimmering
			// https://johanmedestrom.wordpress.com/2016/03/18/opengl-cascaded-shadow-maps/
			// Exactly how it does it, I'm not sure..
			auto shadowMatrix = light_view * ortho_mat;
			auto shadowOrigin = DirectX::SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 1.0f);
			shadowOrigin = DirectX::SimpleMath::Vector4::Transform(shadowOrigin, shadowMatrix);
			shadowOrigin = shadowOrigin * m_shadow_map_resolution / 2.0f;

			auto roundedOrigin = DirectX::SimpleMath::Vector4(
				std::roundf(shadowOrigin.x), 
				std::roundf(shadowOrigin.y), 
				std::roundf(shadowOrigin.z), 
				std::roundf(shadowOrigin.w));
			auto roundOffset = roundedOrigin - shadowOrigin;
			roundOffset = roundOffset * 2.0f / m_shadow_map_resolution;
			roundOffset.z = 0.0f;
			roundOffset.w = 0.0f;

			ortho_mat.m[3][0] += roundOffset.x;
			ortho_mat.m[3][1] += roundOffset.y;
			ortho_mat.m[3][2] += roundOffset.z;
			ortho_mat.m[3][3] += roundOffset.w;

			lastSplitDist = cascade_splits[i];

			//// Store split distance and matrix in cascade
			//cascades[i].splitDepth = (camera.getNearClip() + splitDist * clipRange) * -1.0f;

			DirectX::SimpleMath::Matrix total = light_view * ortho_mat;
			matrices[i] = total;
		}
		
		auto total = matrices[m_cascade];
		m_light_data.view_proj = total;
		m_light_data.view_proj_inv = total.Invert();
		m_light_data.light_direction = DirectX::SimpleMath::Vector4(light_direction);
	}



	// Update light data
	auto single_light_copy = m_copy_bucket.add_command<gfxcommand::CopyToBuffer>(0, 0);
	single_light_copy->buffer = m_per_light_cb;
	single_light_copy->data = &m_light_data;
	single_light_copy->data_size = sizeof(PerLightData);

	// Sort buckets
	{
		auto _ = FrameProfiler::ScopedCPU("Command Bucket Sorting");
		m_opaque_bucket.sort();
		m_shadow_bucket.sort();
	}

	// Upload per frame data to GPU
	gfx::dev->map_copy(m_cb_per_frame, SubresourceData(&m_cb_dat, sizeof(m_cb_dat)));

	// Dispatch miscellaneous per-frame GPU copies
	m_copy_bucket.flush();

	// Dispatch compute work
	m_compute_bucket.flush();

	// Bind per frame data (should be bound to like slot 14 as reserved space)
	gfx::dev->bind_constant_buffer(GLOBAL_PER_FRAME_CB_SLOT, ShaderStage::eVertex, m_cb_per_frame, 0);




	// Geometry Pass
	{
		auto _ = FrameProfiler::Scoped("Geometry Pass");

		gfx::dev->begin_pass(m_gbuffer_res.rp);
		gfx::dev->bind_viewports({ viewports[1] });

		m_opaque_bucket.flush();

		gfx::dev->end_pass();
	}

	// Shadow Pass
	{
		auto _ = FrameProfiler::Scoped("Shadow Pass");

		gfx::dev->begin_pass(m_dir_rp);
		gfx::dev->bind_viewports({ viewports[2] });
		gfx::dev->bind_constant_buffer(2, ShaderStage::eVertex, m_per_light_cb);

		m_shadow_bucket.flush();

		gfx::dev->end_pass();
	}

	// Lightpass (Fullscreen)
	{
		auto _ = FrameProfiler::Scoped("Light Pass");

		gfx::dev->begin_pass(m_lightpass_rp);
		gfx::dev->bind_viewports({ viewports[1] });
		gfx::dev->bind_pipeline(m_lightpass_pipe);
	
		m_gbuffer_res.read_bind(gfx::dev);

		// Directional light can be CB
		// Pointlight and Spotlight should be structured buffers (we'll save for later, implement SDSM first for directional)
		gfx::dev->bind_constant_buffer(1, ShaderStage::ePixel, m_per_light_cb);
		
		//gfx::dev->bind_resource(5, ShaderStage::ePixel, m_rw_splits);
		gfx::dev->bind_resource(6, ShaderStage::ePixel, m_d_32);
		gfx::dev->bind_resource(7, ShaderStage::ePixel, m_dir_d32);

		gfx::dev->draw(6);
		gfx::dev->end_pass();
	}

	// Write to backbuffer for final post-proc (tone mapping and gamma correction)
	{
		auto _ = FrameProfiler::Scoped("Final Fullscreen Pass");
		gfx::dev->begin_pass(m_backbuffer_out_rp);
		gfx::dev->bind_viewports(viewports);
		gfx::dev->bind_pipeline(m_final_pipe);
		gfx::dev->bind_resource(0, ShaderStage::ePixel, m_lightpass_output);
		gfx::dev->draw(6);

		// Draw ImGUI overlay on top of everything
		gfx::imgui->draw();

		gfx::dev->end_pass();
	}


	compute_SDSM();
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
	ImGui::SliderFloat("Lambda", &m_lambda, 0.0f, 1.f);
	ImGui::SliderInt("Cascade", &m_cascade, 0, NUM_CASCADES - 1);
	ImGui::SliderFloat3("Sun Direction", &m_sun_direction.x, -1.f, 1.f);	// y and z lives right after x (check XMFLOAT3)
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
			// discard non .hlsl
			if (std::filesystem::path(entry).extension() != ".hlsl")
				continue;

			shader_filenames.insert(std::filesystem::path(entry).stem().string());

		}
		selected_item = shader_filenames.cbegin()->c_str();
		do_once = false;
	}

	ImGui::Begin("Shader Reloader");
	
	const char* combo_preview_value = selected_item;
	if (ImGui::BeginCombo("Files", combo_preview_value, ImGuiComboFlags_HeightLarge))
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
	world = gfx::dev->create_texture(TextureDesc::make_2d(DXGI_FORMAT_R32G32B32A32_FLOAT, width, height,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
		1, 1));
}

void Renderer::GBuffer::create_rp(GfxDevice* dev, TextureHandle depth)
{
	// geometry pass output
	rp = gfx::dev->create_renderpass(RenderPassDesc(
		{
			albedo,
			normal,
			world
		}, depth));
}

void Renderer::GBuffer::read_bind(GfxDevice* dev)
{
	dev->bind_resource(GBUFFER_ALBEDO_TEXTURE_SLOT, ShaderStage::ePixel, albedo);
	dev->bind_resource(GBUFFER_NORMAL_TEXTURE_SLOT, ShaderStage::ePixel, normal);
	dev->bind_resource(GBUFFER_WORLD_TEXTURE_SLOT, ShaderStage::ePixel, world);
}

void Renderer::GBuffer::free(GfxDevice* dev)
{
	gfx::dev->free_texture(albedo);
	gfx::dev->free_texture(normal);
	gfx::dev->free_texture(world);
}
