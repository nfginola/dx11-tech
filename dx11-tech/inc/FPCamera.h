#pragma once
#include "Camera.h"

class FPCamera final : public Camera
{
public:
	FPCamera(float fov_deg, float aspect_ratio, float near_plane = 0.1f, float far_plane = 100.f, bool reversed_depth = false);
	~FPCamera() = default;


};

