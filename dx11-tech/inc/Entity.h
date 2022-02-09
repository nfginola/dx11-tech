#pragma once
#include "Component.h"
#include "ComponentMapper.h"

#define MAX_COMPONENTS 16

class Entity
{
public:
	Entity();
	~Entity();

	Entity& operator=(const Entity&) = delete;
	Entity(const Entity&) = delete;
	
	template <ComponentType T>
	void add_component(Component* comp);

	template <ComponentType T>
	auto& get_component();

	uint32_t get_active_components() const;

private:
	std::array<Component*, MAX_COMPONENTS> m_components;

	// Bitflags using ComponentType
	uint32_t m_active_component_bits;

};


template<ComponentType T>
inline void Entity::add_component(Component* comp)
{
	// Check that the component given is the right type
	assert((comp->GetBit() & ComponentMapper<T>::bit) == ComponentMapper<T>::bit);

	// Check that the component type doesn't already exist
	assert((m_active_component_bits & ComponentMapper<T>::bit) != ComponentMapper<T>::bit);

	// Add component
	m_components[ComponentMapper<T>::index] = comp;
	m_active_component_bits |= ComponentMapper<T>::bit;
}

template<ComponentType T>
inline auto& Entity::get_component()
{
	// Make sure the component exists!
	assert((m_active_component_bits & ComponentMapper<T>::bit) == ComponentMapper<T>::bit);
	return (*static_cast<ComponentMapper<T>::type*>(m_components[ComponentMapper<T>::index]));
}
