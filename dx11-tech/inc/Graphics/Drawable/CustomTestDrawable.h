#pragma once
#include "Graphics/Drawable/ICustomDrawable.h"

class CustomTestDrawable final : public ICustomDrawable
{
public:
	CustomTestDrawable() = default;
	~CustomTestDrawable() = default;

	void update(float dt) override;
	void submit(class Renderer* renderer, const DirectX::SimpleMath::Matrix& world_mat) override;

	void pre_render() override;
	void on_render() override;

};

