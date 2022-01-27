#include "pch.h"
#include "FPCamera.h"

FPCamera::FPCamera(float fov_deg, float aspect_ratio, float near_plane, float far_plane, bool reversed_depth)
{
	// Create persp mat
	m_proj_mat = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov_deg), aspect_ratio, near_plane, far_plane);

	// Fix reversed depth later (manual perspective mat?)

}
