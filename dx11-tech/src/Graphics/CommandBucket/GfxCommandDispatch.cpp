#include "pch.h"
#include "Graphics/CommandBucket/GfxCommandDispatch.h"
#include "Graphics/API/GfxDevice.h"

// Global dependencies
namespace gfx
{
	extern GfxDevice* dev;
}

void gfxcommand_dispatch::draw(const void* data)
{
	fmt::print("ayaya\n");
}

void gfxcommand_dispatch::copy_to_cbuffer(const void* data)
{



}
