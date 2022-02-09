#include "pch.h"
#include "Entity.h"


Entity::Entity() :
	m_active_component_bits(0)
{
	std::memset(m_components.data(), 0, sizeof(Component*) * MAX_COMPONENTS);
	m_components[ComponentMapper<ComponentType::eTransformComponent>::index] = new TransformComponent();
}

Entity::~Entity()
{
	// Clean up transform component
	auto tr = reinterpret_cast<TransformComponent*>(m_components[ComponentMapper<ComponentType::eTransformComponent>::index]);
	delete tr;
}

uint32_t Entity::get_active_components() const
{
	return m_active_component_bits;
}
