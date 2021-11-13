#include "pch.h"
#include "Camera.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::set_position(float x, float y, float z)
{
	m_position = DirectX::SimpleMath::Vector4(x, y, z, 1.f);
}

void Camera::set_orientation()
{
	assert(false);
	// to implement
}
