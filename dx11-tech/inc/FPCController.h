#pragma once
class FPCController
{
public:
	FPCController(class Input* input) : m_cam(nullptr), m_input(input){};
	~FPCController() = default;

	/*
		May be replaced with FPCamera in the future where FP-Perspective (FPP) inherits from FP
	*/
	void set_camera(class FPPCamera* cam);

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



};

