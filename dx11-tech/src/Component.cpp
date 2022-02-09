#include "pch.h"
#include "Component.h"

TransformComponent& TransformComponent::set_position(float x, float y, float z)
{
	m_pos = DirectX::SimpleMath::Vector3(x, y, z);
	return *this;
}

TransformComponent& TransformComponent::set_scale(float xyz)
{
	m_scale = DirectX::SimpleMath::Vector3(xyz, xyz, xyz);
	return *this;
}

TransformComponent& TransformComponent::set_scale(float x, float y, float z)
{
	m_scale = DirectX::SimpleMath::Vector3(x, y, z);
	return *this;
}

ModelComponent::ModelComponent(const Model* model) :
	m_model(model)
{
}
