#pragma once
#include "Camera.h"

class FPCamera final : public Camera
{
public:
	FPCamera(float fov_deg, float aspect_ratio, float near_plane = 0.1f, float far_plane = 100.f, bool reversed_depth = false);
	~FPCamera() = default;

	void update_orientation(float mouse_x_delta, float mouse_y_delta, float dt);

private:
	float m_yaw = 90.f;
	float m_pitch = 0.f;

	DirectX::SimpleMath::Vector4 m_lookat_pos;
};

