#pragma once

#include "blend2d.h"

struct IRender : BLContext
{
	IRender(BLImage& img) : BLContext(img) {}
};

// IDrawable
// Base interface for anything that might have an effect
// on a drawing context.
// 
struct IDrawable
{
    virtual ~IDrawable() {}

    virtual void draw(IRender& ctx) = 0;
};

struct SVGRenderer : public IRender
{
	SVGRenderer(BLImage& img) : IRender(img) {}
};