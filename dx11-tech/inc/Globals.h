#pragma once

namespace gfx
{
	// API for interacting with D3D11
	extern class GfxDevice* dev;			
	
	// GPU command scope annotator
	extern class GPUAnnotator* annotator;
}

namespace perf
{
	// Frame Profiler
	extern class FrameProfiler* profiler;
}
