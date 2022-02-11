#pragma once
#include "IDrawable.h"

/*
	Interface for fully custom drawing to the geometry buffer.
	This can be specialized classes where you would want some GPU pre-computing (e.g compute shader or render to buffers).
	This is meant to ease implementation of new tech in an isolated manner and always ensuring it gets draw onto as visible geometry.

	Example drawables: Terrain / Water, ...
*/
class ICustomDrawable : virtual public IDrawable	
{
public:
	virtual ~ICustomDrawable() {};

	// GPU pre-rendering work prior to draws
	virtual void pre_render() = 0;

	// On render to e.g geometry buffer
	virtual void on_render() = 0;
};
