#pragma once

#include "blend2d.h"

// IDrawable
// Base interface to things that can draw.
// There is no concept of boundaries or movement
// or container for other things.
// This interface is meant for those things that 
// are not UI elements, but typically static display elements
struct IDrawable
{
	virtual ~IDrawable() {}

	virtual void draw(BLContext & ctx) = 0;
};



