#pragma once
#include "Camera.h"

class PerspectiveCamera final : public Camera
{
public:
	PerspectiveCamera(float fov_deg, float aspect_ratio, float near_plane = 0.1f, float far_plane = 100.f, bool reversed_depth = false);
	~PerspectiveCamera() = default;
};

