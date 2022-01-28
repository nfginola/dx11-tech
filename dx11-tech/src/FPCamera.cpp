#include "..\inc\FPCamera.h"
#include "pch.h"
#include "FPCamera.h"

FPCamera::FPCamera(float fov_deg, float aspect_ratio, float near_plane, float far_plane, bool reversed_depth)
{
	// Create persp mat
	m_proj_mat = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov_deg), aspect_ratio, near_plane, far_plane);

	// Fix reversed depth later (manual perspective mat?)

}

void FPCamera::update_orientation(float mouse_x_delta, float mouse_y_delta, float dt)
{
	/*
	Given h = 1

	Y
	|		  /|
	|		 / |
	|		/  |
	|	   /   |
	|	  /    |
	|	 /     | sin(a)
	|	/	   |
	|  /	   |
	| /\	   |
	|/ a)      |
	----------------- XZ
		cos(a)

	Forward.X = cos(a)
	Forward.Y = sin(a)
	Forward.Z = cos(a)

	Z
	|		  /|
	|		 / |
	|		/  |
	|	   /   |
	|	  /    |
	|	 /     | sin(b)
	|	/	   |
	|  /	   |
	| /\	   |
	|/ b)      |
	----------------- X
		cos(b)

	Forward.X = cos(b)

	Forward.Z = sin(b)

	-------------------------
	Put together all contribution:

	Forward.X = cos(a) * cos(b)
	Forward.Y = sin(a)
	Forward.Z = cos(a) * sin(b)

	--------------------------
	Pitch (positive rotation around X using RH rule)
	Yaw (positive rotation around Y using LH rule)

	a = Pitch
	b = Yaw

	But note that when Yaw (b) = 0, we will point towards the X direction.
	We can set it to Yaw (b) = 90 to point it towards Z by default.

	Additionally, when changing Yaw, the default rotation around Y uses RH rule as
	seen on the second triangle. We can instead decrement the the Yaw to have rotation
	around Y according to LH rule.

	--------------------------

*/

	float sensitivity = 21.f;

	m_yaw -= mouse_x_delta * sensitivity * dt;
	m_pitch += -mouse_y_delta * sensitivity * dt;	// Delta Y is positive downwards by default

	// Restrict 
	if (m_pitch > 89.f)
		m_pitch = 89.f;
	if (m_pitch < -89.f)
		m_pitch = -89.f;

	// Wrap 
	if (m_yaw > 360.f)
		m_yaw = 0.f;
	if (m_yaw < -360.f)
		m_yaw = 0.f;

	//fmt::print("yaw: {}, pitch: {}\n", yaw, pitch);
	
	// Direction
	m_lookat_pos.x = cos(DirectX::XMConvertToRadians(m_pitch)) * cos(DirectX::XMConvertToRadians(m_yaw));
	m_lookat_pos.y = sin(DirectX::XMConvertToRadians(m_pitch));
	m_lookat_pos.z = cos(DirectX::XMConvertToRadians(m_pitch)) * sin(DirectX::XMConvertToRadians(m_yaw));
	m_lookat_pos.Normalize();
	
	// Compute look at pos according to direction
	m_lookat_pos += m_position;

	//fmt::print("look pos: [{}, {}, {}]\n", x, y, z);

	m_view_mat = DirectX::XMMatrixLookAtLH(
		m_position,
		m_lookat_pos,
		s_world_up);
}

