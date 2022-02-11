#include "pch.h"
#include "Graphics/Renderer.h"
#include "Graphics/Drawable/CustomTestDrawable.h"

void CustomTestDrawable::update(float dt)
{
	fmt::print("CPU update!\n");
}

void CustomTestDrawable::submit(class Renderer* renderer, const DirectX::SimpleMath::Matrix& world_mat)
{
	fmt::print("Submitting to GPU!\n");
	//renderer->submit(this);
}

void CustomTestDrawable::pre_render()
{
	fmt::print("Pre Render!\n");
}

void CustomTestDrawable::on_render()
{
	fmt::print("On Render!\n");
}
