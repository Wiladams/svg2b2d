#pragma once

#include "bspanutil.h"
#include "xmlscan.h"

namespace svg2b2d
{
    // Determine at runtime if the CPU is little-endian (intel standard)
    static constexpr bool isLE() noexcept { int i = 1; return (int)*((unsigned char*)&i) == 1; }
    static constexpr bool isBE() noexcept { return !isLE(); }

    // Just a few useful routines
    static constexpr double radians(double a) { return a * 0.017453292519943295; }     // given degrees, return radians
    static constexpr double degrees(double a) { return a * 57.29577951308232; }        // given radians, return degrees

    static constexpr double clamp(double a, double min_, double max_) { if (a < min_) return min_; if (a > max_) return max_; return a; }
}

namespace svg2b2d
{

    enum SVGDimensionUnits
    {
        SVG_UNITS_USER,
        SVG_UNITS_PX,
        SVG_UNITS_PT,
        SVG_UNITS_PC,
        SVG_UNITS_MM,
        SVG_UNITS_CM,
        SVG_UNITS_IN,
        SVG_UNITS_PERCENT,
        SVG_UNITS_EM,
        SVG_UNITS_EX
    };
    
    // Turn a units indicator into an enum
    static SVGDimensionUnits parseDimensionUnits(const ByteSpan& units)
    {
        // if the chunk is blank, then return user units
        if (!units)
            return SVG_UNITS_USER;

        if (units[0] == 'p' && units[1] == 'x')
            return SVG_UNITS_PX;
        else if (units[0] == 'p' && units[1] == 't')
            return SVG_UNITS_PT;
        else if (units[0] == 'p' && units[1] == 'c')
            return SVG_UNITS_PC;
        else if (units[0] == 'm' && units[1] == 'm')
            return SVG_UNITS_MM;
        else if (units[0] == 'c' && units[1] == 'm')
            return SVG_UNITS_CM;
        else if (units[0] == 'i' && units[1] == 'n')
            return SVG_UNITS_IN;
        else if (units[0] == '%')
            return SVG_UNITS_PERCENT;
        else if (units[0] == 'e' && units[1] == 'm')
            return SVG_UNITS_EM;
        else if (units[0] == 'e' && units[1] == 'x')
            return SVG_UNITS_EX;

        return SVG_UNITS_USER;
    }

    struct SVGDimension {
        double fValue;
        SVGDimensionUnits fUnits;

        SVGDimension()
            : fValue(0.0f)
            , fUnits(SVGDimensionUnits::SVG_UNITS_USER)
        {
        }

        SVGDimension(const SVGDimension& other)
            :fValue(other.fValue)
            , fUnits(other.fUnits)
        {}

        SVGDimension(double value, SVGDimensionUnits units)
            : fValue(value)
            , fUnits(units)
        {
        }

        SVGDimension& operator=(const SVGDimension& rhs)
        {
            fValue = rhs.fValue;
            fUnits = rhs.fUnits;
        }
        
        double value() const { return fValue; }
        SVGDimensionUnits units() const { return fUnits; }
        double calculatePixels(double length = 1.0, double orig = 0, double dpi = 96)
        {
            switch (fUnits) {
            case SVG_UNITS_USER:		return fValue;
            case SVG_UNITS_PX:			return fValue;
            case SVG_UNITS_PT:			return fValue / 72.0f * dpi;
            case SVG_UNITS_PC:			return fValue / 6.0f * dpi;
            case SVG_UNITS_MM:			return fValue / 25.4f * dpi;
            case SVG_UNITS_CM:			return fValue / 2.54f * dpi;
            case SVG_UNITS_IN:			return fValue * dpi;
                //case SVG_UNITS_EM:			return fValue * attr.fontSize;
                //case SVG_UNITS_EX:			return fValue * attr.fontSize * 0.52f; // x-height of Helvetica.
            case SVG_UNITS_PERCENT:	return orig + fValue / 100.0f * length;

            }

            return fValue;
        }

        void loadSelfFromChunk(const ByteSpan& inChunk)
        {
            ByteSpan s = inChunk;
            ByteSpan numChunk;
            auto nextPart = scanNumber(s, numChunk);
            fValue = chunk_to_double(numChunk);
            fUnits = parseDimensionUnits(nextPart);
        }
    };

    static SVGDimension parseDimension(const ByteSpan& inChunk)
    {
        SVGDimension dim{};
        dim.loadSelfFromChunk(inChunk);

        return dim;
    }
    
}