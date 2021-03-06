#pragma once

namespace gfx
{
	extern class GfxDevice* dev;				// API for interacting with D3D11
	extern class GPUAnnotator* annotator;		// GPU command scope annotator
	extern class ImGuiDevice* imgui;
	extern class DiskTextureManager* tex_mgr;
	extern class MaterialManager* mat_mgr;
	extern class ModelManager* model_mgr;
	extern class Renderer* rend;
}

namespace perf
{
	extern class CPUProfiler* cpu_profiler;		// All purpose CPU profiler (not actively used anywhere for now, everything goes through FrameProfiler)
	extern class FrameProfiler* profiler;		// Frame profiler
}
