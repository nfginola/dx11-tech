#include "pch.h"
#include "Graphics/CommandBucket/GfxCommand.h"
#include "Graphics/CommandBucket/GfxCommandDispatch.h"

/*
	Assign dispatch functions to commands
*/

const GfxCommandDispatch gfxcommand::Draw::DISPATCH = gfxcommand_dispatch::draw;