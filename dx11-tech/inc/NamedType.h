#pragma once

// Strongly typed
template <typename T, typename Phantom>
class NamedType
{
public:
	explicit NamedType(T const& value) : m_value(value) {}
	T get() { return m_value; }
private:
	T m_value;
};
