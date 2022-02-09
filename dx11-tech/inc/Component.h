#pragma once
#include "SimpleMath.h"

static constexpr int MAX_COMPONENTS = 16;

enum ComponentType
{
	eTransformComponent = 1,
	eModelComponent
};



class Component
{
public:


protected:
	Component() = default;
	virtual ~Component() = default;

};

class TransformComponent : public Component
{
public:
	TransformComponent() = default;
	~TransformComponent() = default;

	TransformComponent& set_position(float x, float y, float z);

	TransformComponent& set_scale(float xyz);
	TransformComponent& set_scale(float x, float y, float z);

	void set_orientation();	// todo

private:
	DirectX::SimpleMath::Vector3 m_pos;
	DirectX::SimpleMath::Quaternion m_orientation;
	DirectX::SimpleMath::Vector3 m_scale = DirectX::SimpleMath::Vector3(1.f, 1.f, 1.f);

};

class ModelComponent : public Component
{
public:
	ModelComponent() = delete;
	ModelComponent(const class Model* model);
	~ModelComponent() = default;

private:
	const Model* m_model;
};
