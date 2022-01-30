#include "pch.h"
#include "FPCController.h"

#include "FPPCamera.h"
#include "Input.h"

void FPCController::set_camera(FPPCamera* cam)
{
	m_cam = cam;
}

Camera* FPCController::get_active_camera()
{
	return m_cam;
}

void FPCController::update(float dt)
{
	// Reset state
	m_fwd_state = 0.f;
	m_up_state = 0.f;
	m_right_state = 0.f;

	if (m_input->rmb_down())
	{
		m_input->set_mouse_mode(MouseMode::Relative);	// Hide mouse
		auto [x_delta, y_delta] = m_input->get_mouse_delta();
		m_cam->update_orientation((float)x_delta, (float)y_delta, dt);
	}

	// Unhide mouse
	if (m_input->rmb_released())		m_input->set_mouse_mode(MouseMode::Absolute);

	/*
		[-1, 1] is the constraint of the direction arguments to update_position()
	*/
	if (m_input->key_down(Keys::W))			utils::constrained_incr(m_fwd_state, -1.f, 1.f);
	if (m_input->key_down(Keys::S))			utils::constrained_decr(m_fwd_state, -1.f, 1.f);
	if (m_input->key_down(Keys::D))			utils::constrained_incr(m_right_state, -1.f, 1.f);
	if (m_input->key_down(Keys::A))			utils::constrained_decr(m_right_state, -1.f, 1.f);
	if (m_input->key_down(Keys::Space))		utils::constrained_incr(m_up_state, -1.f, 1.f);
	if (m_input->key_down(Keys::LeftShift))	utils::constrained_decr(m_up_state, -1.f, 1.f);

	// [0, 120, 240, ..] --> [0, 1, 2, ..]
	auto scroll_val = (m_input->get_scroll_value() / 120.f) * 1.f;
	auto final_speed = utils::constrained_add(m_init_speed, scroll_val, m_min_speed, m_max_speed);
	m_cam->set_speed(final_speed);

	m_cam->set_sensitivity(m_mouse_sens);

	m_cam->update_position(m_right_state, m_up_state, m_fwd_state, dt);

	m_cam->update_matrices();
}
