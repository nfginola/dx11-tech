#include "pch.h"
#include "Camera.h"

Camera::Camera()
{
	// Create default view mat
	m_view_mat = DirectX::XMMatrixLookAtLH(
		DirectX::SimpleMath::Vector4(0.f, 0.f, -5.f, 1.f),
		DirectX::SimpleMath::Vector4(0.f, 0.f, 1.f, 1.f),	// Start at (0, 0, -5), look forward
		s_world_up);
}

Camera::~Camera()
{
}

void Camera::set_position(float x, float y, float z)
{
	m_position = DirectX::SimpleMath::Vector4(x, y, z, 1.f);
	m_view_mat = DirectX::XMMatrixLookAtLH(m_position, DirectX::SimpleMath::Vector4(0.f, 0.f, 0.f, 1.f), s_world_up);
}

void Camera::set_orientation()
{
	assert(false);
	// to implement
}

const DirectX::SimpleMath::Vector4& Camera::get_position() const
{
	return m_position;
}

const DirectX::SimpleMath::Matrix& Camera::get_view_mat() const
{
	return m_view_mat;
}

const DirectX::SimpleMath::Matrix& Camera::get_proj_mat() const
{
	return m_proj_mat;
}
