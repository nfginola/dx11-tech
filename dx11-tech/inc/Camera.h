#pragma once

class Camera
{
public:
	Camera();
	virtual ~Camera();

	Camera& operator=(const Camera&) = delete;
	Camera(const Camera&) = delete;
	
	void set_position(float x, float y, float z);
	void set_orientation();

	virtual const DirectX::SimpleMath::Vector4& get_position() const = 0;
	virtual DirectX::SimpleMath::Matrix get_view_mat() const = 0;
	virtual DirectX::SimpleMath::Matrix get_proj_mat() const = 0;

protected:
	// LH system
	static constexpr DirectX::SimpleMath::Vector3 s_world_up = { 0.f, 1.f, 0.f };
	static constexpr DirectX::SimpleMath::Vector3 s_world_right = { 1.f, 0.f, 0.f };
	static constexpr DirectX::SimpleMath::Vector3 s_world_forward = { 0.f, 0.f, 1.f };

	DirectX::SimpleMath::Vector4 m_position;


};

