#pragma once
#include "Component.h"

template <ComponentType T>
struct ComponentMapper;

template <>
struct ComponentMapper<ComponentType::eTransformComponent>
{
	using type = TransformComponent;
	static constexpr uint32_t bit = ComponentType::eTransformComponent;
	static constexpr int index = bit - 1;		// Enum starts at 1, so we map the enum directly to array indices
};

template <>
struct ComponentMapper<ComponentType::eModelComponent>
{
	using type = ModelComponent;
	static constexpr uint32_t bit = ComponentType::eModelComponent;
	static constexpr int index = bit - 1;
};

