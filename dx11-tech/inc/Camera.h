#pragma once

class Camera
{
public:
	virtual ~Camera();
	
	void set_position(float x, float y, float z);
	void set_orientation();

	const DirectX::SimpleMath::Vector4& get_position() const;
	virtual const DirectX::SimpleMath::Matrix& get_view_mat() const;
	virtual const DirectX::SimpleMath::Matrix& get_proj_mat() const;

protected:
	Camera();

	// LH system
	static constexpr DirectX::SimpleMath::Vector4 s_world_up = { 0.f, 1.f, 0.f, 0.f };
	static constexpr DirectX::SimpleMath::Vector4 s_world_right = { 1.f, 0.f, 0.f, 0.f };
	static constexpr DirectX::SimpleMath::Vector4 s_world_forward = { 0.f, 0.f, 1.f, 0.f };

	DirectX::SimpleMath::Vector4 m_position;
	DirectX::SimpleMath::Matrix m_view_mat, m_proj_mat;

	DirectX::SimpleMath::Vector4 m_lookat_pos;



};

