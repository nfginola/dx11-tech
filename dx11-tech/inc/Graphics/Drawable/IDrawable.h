#pragma once

class IDrawable
{
public:
	virtual ~IDrawable() {};

	// Update CPU state of drawable
	virtual void update(float dt) = 0;	

	// Submit drawable to GPU
	virtual void submit(class Renderer* renderer, const DirectX::SimpleMath::Matrix& world_mat) = 0;
};
