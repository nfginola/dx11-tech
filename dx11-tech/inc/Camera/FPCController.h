#pragma once
class FPCController
{
public:
	FPCController(class Input* input);
	~FPCController() = default;

	/*
		May be replaced with FPCamera in the future where FP-Perspective (FPP) inherits from FP
	*/
	void set_camera(class FPPCamera* cam);

	class Camera* get_active_camera();

	void update(float dt);

private:
	/*
		Note that the Camera that is non-owned below should have its own
		position and orientation, which is persistent should this controller
		possess another Camera.
	*/

	FPPCamera* m_cam;
	Input* m_input;

	// State to keep track of the direction controller
	float m_fwd_state = 0.f;
	float m_up_state = 0.f;
	float m_right_state = 0.f;

	// Base speed
	float m_init_speed = 7.f;
	float m_curr_speed = m_init_speed;
	float m_min_speed = 1.f;
	float m_max_speed = 11.f;

	// Sensitivity
	float m_mouse_sens = 12.3f;
	



};
