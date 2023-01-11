#pragma once


#include "bspanutil.h"

#include "svgcolors.h"
#include "xmlscan.h"


#include <memory>
#include <vector>
#include <cstdint>		// uint8_t, etc
#include <cstddef>		// nullptr_t, ptrdiff_t, size_t




namespace svg2b2d
{
    // Determine at runtime if the CPU is little-endian (intel standard)
    static inline bool isLE() noexcept { int i = 1; return (int)*((unsigned char*)&i) == 1; }
    static inline bool isBE() noexcept { return !isLE(); }
}


// IDrawable
// Base interface to things that can draw.
// There is no concept of boundaries or movement
// or container for other things.
// This interface is meant for those things that 
// are not UI elements, but typically static display elements
struct IDrawable
{
    virtual ~IDrawable() {}

    virtual void draw(BLContext& ctx) = 0;
};



namespace svg2b2d {
    // given degrees, return radians
    constexpr double radians(double a) { return a * 0.017453292519943295; }
    
    // given radians, return degrees
    constexpr double degrees(double a) { return a * 57.29577951308232; }
    
    inline int Min(double a, double b) { return (a < b) ? a : b; }
    inline int Max(double a, double b) { return (a > b) ? a : b; }
    inline int clamp(double a, double min_, double max_) { return Min(Max(a,min_), max_); }
    
    // SVG Element Attributes are of fixed types
// The SVGAttributeKind enum defines the types
// https://www.w3.org/TR/SVG2/attindex.html#PresentationAttributes
    enum SVGAttributeKind
    {
        SVG_ATTR_KIND_INVALID = 0
        , SVG_ATTR_KIND_CHUNK               // If there is no better representation

        , SVG_ATTR_KIND_NUMBER              // floating point number
		, SVG_ATTR_KIND_NUMBERORPERCENT     // floating point number or percentage, range [0..1]
        , SVG_ATTR_KIND_DIMENSION           // value/units
		, SVG_ATTR_KIND_COLOR               // color
        , SVG_ATTR_KIND_PAINT               // color, gradient, pattern
        , SVG_ATTR_KIND_TRANSFORM           // matrix
		, SVG_ATTR_KIND_ENUM 			    // enumeration of known (typically string) values
        
        , SVG_ATTR_KIND_BOOL                // bool
        , SVG_ATTR_KIND_INT                 // int
        , SVG_ATTR_KIND_STRING              // string
        , SVG_ATTR_KIND_POINTS              // points for a poly
        , SVG_ATTR_KIND_PATH                // path data
    };


    // Enums that represent choices for individual attributes
    // 
    enum SVGClipRule
    {
        SVG_CLIP_RULE_NONZERO = 0
        , SVG_CLIP_RULE_EVENODD
    };

    enum SVGColorInterpolation {
		SVG_COLOR_INTERPOLATION_AUTO
		, SVG_COLOR_INTERPOLATION_SRGB
		, SVG_COLOR_INTERPOLATION_LINEARRGB
    };
    
    // Various Attributes to be found in an SVG image
    enum SVGAlignType {
        SVG_ALIGN_NONE = 0,
        SVG_ALIGN_MIN = 0,
        SVG_ALIGN_MID = 1,
        SVG_ALIGN_MAX = 2,
        SVG_ALIGN_MEET = 1,
        SVG_ALIGN_SLICE = 2,
    };

    enum SVGspreadType {
        SVG_SPREAD_PAD = 0,
        SVG_SPREAD_REFLECT = 1,
        SVG_SPREAD_REPEAT = 2
    };

    enum SVGGradientUnits {
        SVG_USER_SPACE = 0,
        SVG_OBJECT_SPACE = 1
    };



    enum SVGflags {
        SVG_FLAGS_VISIBLE = 0x01
    };



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
}

//
// PathContour - smallest addressable geometry
// PathSegment - a collection of contours
// Path        - a complete set of geometry
// Shape        - Geometry + drawing attributes
//

namespace svg2b2d
{
    enum SVGlineJoin {
        SVG_JOIN_MITER_CLIP = 0,
        SVG_JOIN_MITER_BEVEL = 1,
        SVG_JOIN_MITER_ROUND = 2,
        SVG_JOIN_BEVEL = 3,
        SVG_JOIN_ROUND = 4,
        // SVG_JOIN_ARCS = 3,


    };

    enum SVGlineCap {
        SVG_CAP_BUTT = 0,
        SVG_CAP_SQUARE = 1,
        SVG_CAP_ROUND = 2,

    };

    enum SVGfillRule {
        SVG_FILLRULE_NONZERO = 0,
        SVG_FILLRULE_EVENODD = 1
    };

    enum SVGpaintType {
        SVG_PAINT_NONE = 0,
        SVG_PAINT_COLOR = 1,
        SVG_PAINT_LINEAR_GRADIENT = 2,
        SVG_PAINT_RADIAL_GRADIENT = 3
    };

    // Shaper contour Commands
    // Origin from SVG path commands
    // M - move       (M, m)
    // L - line       (L, l, H, h, V, v)
    // C - cubic      (C, c, S, s)
    // Q - quad       (Q, q, T, t)
    // A - ellipticArc  (A, a)
    // Z - close        (Z, z)
    enum class SegmentKind : uint8_t
    {
        INVALID = 0
        , MoveTo = 'M'
        , MoveBy = 'm'
        , LineTo = 'L'
        , LineBy = 'l'
        , HLineTo = 'H'
        , HLineBy = 'h'
        , VLineTo = 'V'
        , VLineBy = 'v'
        , CubicTo = 'C'
        , CubicBy = 'c'
        , SCubicTo = 'S'
        , SCubicBy = 's'
        , QuadTo = 'Q'
        , QuadBy = 'q'
        , SQuadTo = 'T'
        , SQuadBy = 't'
        , ArcTo = 'A'
        , ArcBy = 'a'
        , CloseTo = 'Z'
        , CloseBy = 'z'
    };

    enum class ShapeKind : uint8_t
    {
        INVALID = 0
        , Line = 1
        , Rect = 2
        , Circle = 3
        , Ellipse = 4
        , Polyline = 5
        , Polygon = 6
        , Path = 7
    };



    //
    // A Path is comprised of a number of PathSegment structures
    // Each PathSegment begins with a SegmentKind, followed by a series
    // of numbers appropriate for that segment kind. 
    //
    struct PathSegment
    {
        SegmentKind fCommand{ SegmentKind::INVALID };
        std::vector<double> fNumbers{};

        PathSegment() { ; }
        PathSegment(SegmentKind aKind) :fCommand(aKind) { ; }
        PathSegment(const PathSegment& other)
            :fCommand(other.fCommand)
            , fNumbers(other.fNumbers)
        {}

        void addNumber(double aNumber) { fNumbers.push_back(aNumber); }
        void addPoint(double x, double y) { fNumbers.push_back(x); fNumbers.push_back(y); }
    };





    // Parse a number which may have units after it
    //   1.2em
    // -1.0E2em
    // 2.34ex
    // -2.34e3M10,20
    // 
    // By the end of this routine, the numchunk represents the range of the 
    // captured number.
    // 
    // The returned chunk represents what comes next, and can be used
    // to continue scanning the original inChunk
    //
    // Note:  We assume here that the inChunk is already positioned at the start
    // of a number (including +/- sign), with no leading whitespace

    static ByteSpan scanNumber(const ByteSpan& inChunk, ByteSpan& numchunk)
    {
        static charset digitChars("0123456789");                   // only digits

        ByteSpan s = inChunk;
        numchunk = inChunk;
        numchunk.fEnd = inChunk.fStart;


        // sign
        if (*s == '-' || *s == '+') {
            s++;
            numchunk.fEnd = s.fStart;
        }

        // integer part
        while (s && digitChars[*s]) {
            s++;
            numchunk.fEnd = s.fStart;
        }

        if (*s == '.') {
            // decimal point
            s++;
            numchunk.fEnd = s.fStart;

            // fraction part
            while (s && digitChars[*s]) {
                s++;
                numchunk.fEnd = s.fStart;
            }
        }

        // exponent
        // but it could be units (em, ex)
        if ((*s == 'e' || *s == 'E') && (s[1] != 'm' && s[1] != 'x'))
        {
            s++;
            numchunk.fEnd = s.fStart;

            // Might be a sign
            if (*s == '-' || *s == '+') {
                s++;
                numchunk.fEnd = s.fStart;
            }

            // Get any remaining digits
            while (s && digitChars[*s]) {
                s++;
                numchunk.fEnd = s.fStart;
            }
        }

        return s;
    }


    //
    // collectNumbers
    // Take a list of numbers and tokenize them into a vector of floats
    // Numbers are separated by whitespace, commas, or semicolons
    //
    static void collectNumbers(const ByteSpan& chunk, std::vector<float>& numbers)
    {
        static svg2b2d::charset whitespaceChars(",;\t\n\f\r ");          // whitespace found in paths
        static svg2b2d::charset numberChars("0123456789.+-eE");         // digits, symbols, and letters found in numbers


        ByteSpan s = chunk;

        while (s)
        {
            // ignore whitespace
            while (s && whitespaceChars[*s])
                s++;

            if (!s)
                break;

            if (numberChars[*s])
            {
                ByteSpan numChunk = s;

                while (numberChars[*s])
                {
                    s++;
                    numChunk.fEnd = s.fStart;
                }

                float afloat = 0;
                afloat = toNumber(numChunk);
                //std::from_chars((const char*)numChunk.fStart, (const char*)numChunk.fEnd, afloat);

                numbers.push_back(afloat);
            }
        }
    }

    //
    // tokenizePath
    // Given a string, representing a series of path segments, turn the string
    // into a vector of those path segments.
    // 
    // This gives us a structure that can then be turned into other forms, such 
    // as graphic objects.
    // 
    // The syntax of the commands is that of the SVG path object 'd' attribute
    //


    static void tokenizePath(const ByteSpan& chunk, std::vector<PathSegment>& commands)
    {
        static svg2b2d::charset whitespaceChars(",\t\n\f\r ");          // whitespace found in paths
        static svg2b2d::charset commandChars("mMlLhHvVcCqQsStTaAzZ");   // set of characters used for commands
        static svg2b2d::charset numberChars("0123456789.+-eE");         // digits, symbols, and letters found in numbers
        static svg2b2d::charset leadingChars("0123456789.+-");          // digits, symbols, and letters found in numbers
        static svg2b2d::charset digitChars("0123456789");                   // only digits

        // Use a ByteSpan as a cursor on the input
        ByteSpan s = chunk;

        while (s)
        {
            // ignore leading whitespace
            while (s && whitespaceChars[*s])
                s++;

            // See what we've got at this point
            // it's either in commandChars, if we in the START state
            if (commandChars[*s])
            {
                switch (*s) {
                default:
                    //printf("CMD: %c\n", *s);
                    svg2b2d::PathSegment cmd{};
                    cmd.fCommand = svg2b2d::SegmentKind(*s);
                    commands.push_back(cmd);
                    s++;
                }

                continue;
            }

            // or it's something related to a number (+,-,digit,.)
            if (leadingChars[*s])
            {
                // Start with the number chunk being empty
                // expand only if we have a valid number
                ByteSpan numChunk{};

                s = scanNumber(s, numChunk);

                // If we have a number, add it to the last command
                if (numChunk)
                {
                    float afloat = 0;
                    afloat = toNumber(numChunk);
                    //std::from_chars((const char*)numChunk.fStart, (const char*)numChunk.fEnd, afloat);
                    commands.back().addNumber(afloat);
                    //printf("  %3.5f\n", afloat);
                }

            }
        }
    }



}

namespace svg2b2d
{
    //
    // PathBuilder
    // This object takes a series of path segments and turns them into a BLPath object
    //
    struct PathBuilder
    {
    public:


        BLPoint fLastStart{};
        SegmentKind fLastCommand{};

        BLPath fWorkingPath{};


    public:
        BLPath& getPath() { return fWorkingPath; }

        const BLPoint lastPosition() noexcept
        {
            BLPoint apoint{};
            fWorkingPath.getLastVertex(&apoint);

            return apoint;
        }

        // In case there's anything we want to do 
        // at the end of each path segment
        // indicated by a M, m, z, or Z
        void finishWorking()
        {}

        // SVG - M
        // add working path to main path
        // reset the working path to empty
        // establish a new initial position
        void moveTo(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 2) {
                printf("moveTo - Rejected: %zd\n", cmd.fNumbers.size());
                return;
            }

            finishWorking();

            fWorkingPath.moveTo(cmd.fNumbers[0], cmd.fNumbers[1]);
            fLastStart = { cmd.fNumbers[0], cmd.fNumbers[1] };
            //lastPosition();

            // perform absolute lineTo on working path
            // if there are more nunbers
            if (cmd.fNumbers.size() > 2)
            {
                //printf("EXTENDED moveTo\n");

                for (size_t i = 2; i < cmd.fNumbers.size(); i += 2)
                {
                    fWorkingPath.lineTo(cmd.fNumbers[i], cmd.fNumbers[i + 1]);
                }
            }

        }


        // SVG - m
        void moveBy(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 2) {
                printf("moveBy - Rejected: %zd\n", cmd.fNumbers.size());
                return;
            }

            fWorkingPath.moveTo(lastPosition().x + cmd.fNumbers[0], lastPosition().y + cmd.fNumbers[1]);

            // perform relative lineBy on working path
            // if there are more nunbers
            if (cmd.fNumbers.size() > 2)
            {
                //printf("EXTENDED MOVEBY\n");

                for (size_t i = 2; i < cmd.fNumbers.size(); i += 2)
                {
                    fWorkingPath.lineTo(lastPosition().x + cmd.fNumbers[i], lastPosition().y + cmd.fNumbers[i + 1]);
                }
            }
        }

        // Add a line, using current point as first endpoint
        // SVG - L
        void lineTo(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 2)
            {
                printf("lineTo - Rejected: %zd\n", cmd.fNumbers.size());

                return;
            }


            fWorkingPath.lineTo(cmd.fNumbers[0], cmd.fNumbers[1]);


            if (cmd.fNumbers.size() > 2)
            {
                //printf("EXTENDED lineTo\n");

                for (size_t i = 2; i < cmd.fNumbers.size(); i += 2)
                {
                    fWorkingPath.lineTo(cmd.fNumbers[i], cmd.fNumbers[i + 1]);
                }
            }

        }


        // Add a line using relative coordinates
        //SVG - l
        void lineBy(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 2)
            {
                printf("lineBy - Rejected: %zd\n", cmd.fNumbers.size());

                return;
            }

            fWorkingPath.lineTo(lastPosition().x + cmd.fNumbers[0], lastPosition().y + cmd.fNumbers[1]);

            if (cmd.fNumbers.size() > 2)
            {
                //printf("EXTENDED lineBy\n");

                for (size_t i = 2; i < cmd.fNumbers.size(); i += 2)
                {
                    fWorkingPath.lineTo(lastPosition().x + cmd.fNumbers[i], lastPosition().y + cmd.fNumbers[i + 1]);
                }
            }
        }


        // Add horizontal line, using only x coordinate added to existing point
        // SVG - H
        void hLineTo(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 1)
            {
                printf("hLineTo - Rejected: %zd\n", cmd.fNumbers.size());

                return;
            }

            fWorkingPath.lineTo(cmd.fNumbers[0], lastPosition().y);

            if (cmd.fNumbers.size() > 1)
            {
                //printf("EXTENDED hLineTo\n");

                for (size_t i = 1; i < cmd.fNumbers.size(); i++)
                {
                    fWorkingPath.lineTo(cmd.fNumbers[i], lastPosition().y);
                }
            }
        }

        // SVG - h
        void hLineBy(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 1) {
                printf("hLineBy - Rejected: %zd\n", cmd.fNumbers.size());

                return;
            }

            fWorkingPath.lineTo(lastPosition().x + cmd.fNumbers[0], lastPosition().y);

            if (cmd.fNumbers.size() > 1)
            {
                //printf("EXTENDED hLineBy\n");

                for (size_t i = 1; i < cmd.fNumbers.size(); i++)
                {
                    fWorkingPath.lineTo(lastPosition().x + cmd.fNumbers[i], lastPosition().y);
                }
            }
        }

        // SVG - V
        void vLineTo(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 1) {
                printf("vLineTo - Rejected: %zd\n", cmd.fNumbers.size());

                return;
            }

            fWorkingPath.lineTo(lastPosition().x, cmd.fNumbers[0]);

            if (cmd.fNumbers.size() > 1)
            {
                //printf("EXTENDED vLineTo\n");

                for (size_t i = 1; i < cmd.fNumbers.size(); i++)
                {
                    fWorkingPath.lineTo(lastPosition().x, cmd.fNumbers[i]);
                }
            }
        }

        // SVG - v
        void vLineBy(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 1) {
                printf("vLineBy - Rejected: %zd\n", cmd.fNumbers.size());

                return;
            }

            fWorkingPath.lineTo(lastPosition().x, lastPosition().y + cmd.fNumbers[0]);

            if (cmd.fNumbers.size() > 1)
            {
                //printf("EXTENDED vLineBy\n");
                for (size_t i = 1; i < cmd.fNumbers.size(); i++)
                {
                    fWorkingPath.lineTo(lastPosition().x, lastPosition().y + cmd.fNumbers[i]);
                }
            }
        }


        // SVG - Q
        // Quadratic Bezier curve
        // consumes 2 points (4 numbers)
        void quadTo(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 4) {
                printf("quadTo - Rejected: %zd\n", cmd.fNumbers.size());

                return;
            }

            fWorkingPath.quadTo(cmd.fNumbers[0], cmd.fNumbers[1], cmd.fNumbers[2], cmd.fNumbers[3]);

            if (cmd.fNumbers.size() > 4)
            {
                //printf("EXTENDED quadTo\n");
                for (size_t i = 4; i < cmd.fNumbers.size(); i += 4)
                {
                    fWorkingPath.quadTo(cmd.fNumbers[i], cmd.fNumbers[i + 1], cmd.fNumbers[i + 2], cmd.fNumbers[i + 3]);

                }
            }
        }

        // SVG - q
        // Quadratic Bezier curve, relative coordinates
        void quadBy(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 4) {
                printf("quadBy - Rejected: %zd\n", cmd.fNumbers.size());

                return;
            }

            fWorkingPath.quadTo(lastPosition().x + cmd.fNumbers[0], lastPosition().y + cmd.fNumbers[1], lastPosition().x + cmd.fNumbers[2], lastPosition().y + cmd.fNumbers[3]);

            if (cmd.fNumbers.size() > 4)
            {
                //printf("EXTENDED quadBy\n");
                for (size_t i = 4; i < cmd.fNumbers.size(); i += 4)
                {
                    fWorkingPath.quadTo(lastPosition().x + cmd.fNumbers[i], lastPosition().y + cmd.fNumbers[i + 1], lastPosition().x + cmd.fNumbers[i + 2], lastPosition().y + cmd.fNumbers[i + 3]);
                }
            }
        }

        // SVG - T
        // Smooth quadratic Bezier curve
        void smoothQuadTo(const PathSegment& cmd)
        {
            //printf("== NYI - smoothQuadTo ==\n");

            if (cmd.fNumbers.size() < 2) {
                printf("smoothQuadTo - Rejected: %zd\n", cmd.fNumbers.size());

                return;
            }

            fWorkingPath.smoothQuadTo(cmd.fNumbers[0], cmd.fNumbers[1]);

            if (cmd.fNumbers.size() > 2)
            {
                //printf("EXTENDED smoothQuadTo\n");
                for (size_t i = 2; i < cmd.fNumbers.size(); i += 2)
                {
                    fWorkingPath.smoothQuadTo(cmd.fNumbers[i], cmd.fNumbers[i + 1]);
                }
            }
        }

        // SVG - t
        // Smooth quadratic Bezier curve, relative coordinates
        void smoothQuadBy(const PathSegment& cmd)
        {

            if (cmd.fNumbers.size() < 2) {
                printf("smoothQuadBy - Rejected: %zd\n", cmd.fNumbers.size());

                return;
            }

            fWorkingPath.smoothQuadTo(lastPosition().x + cmd.fNumbers[0], lastPosition().y + cmd.fNumbers[1]);

            if (cmd.fNumbers.size() > 2)
            {
                //printf("EXTENDED smoothQuadBy\n");
                for (size_t i = 2; i < cmd.fNumbers.size(); i += 2)
                {
                    fWorkingPath.smoothQuadTo(lastPosition().x + cmd.fNumbers[i], lastPosition().y + cmd.fNumbers[i + 1]);
                }
            }
        }

        // SVG - C
        // Cubic Bezier curve
        void cubicTo(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 6) {
                printf("cubicTo - Rejected: %zd\n", cmd.fNumbers.size());

                return;
            }

            fWorkingPath.cubicTo(BLPoint(cmd.fNumbers[0], cmd.fNumbers[1]), BLPoint(cmd.fNumbers[2], cmd.fNumbers[3]), BLPoint(cmd.fNumbers[4], cmd.fNumbers[5]));

            if (cmd.fNumbers.size() > 6)
            {
                //printf("EXTENDED cubicTo\n");
                for (size_t i = 6; i < cmd.fNumbers.size(); i += 6)
                {
                    fWorkingPath.cubicTo(BLPoint(cmd.fNumbers[i], cmd.fNumbers[i + 1]), BLPoint(cmd.fNumbers[i + 2], cmd.fNumbers[i + 3]), BLPoint(cmd.fNumbers[i + 4], cmd.fNumbers[i + 5]));
                }
            }
        }

        // SVG - c
        // Cubic Bezier curve, relative coordinates
        void cubicBy(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 6) {
                printf("cubicBy - Rejected: %zd\n", cmd.fNumbers.size());
                return;
            }

            fWorkingPath.cubicTo(BLPoint(lastPosition().x + cmd.fNumbers[0], lastPosition().y + cmd.fNumbers[1]), BLPoint(lastPosition().x + cmd.fNumbers[2], lastPosition().y + cmd.fNumbers[3]), BLPoint(lastPosition().x + cmd.fNumbers[4], lastPosition().y + cmd.fNumbers[5]));

            if (cmd.fNumbers.size() > 6)
            {
                //printf("EXTENDED cubicBy\n");
                for (size_t i = 6; i < cmd.fNumbers.size(); i += 6)
                {
                    fWorkingPath.cubicTo(BLPoint(lastPosition().x + cmd.fNumbers[i], lastPosition().y + cmd.fNumbers[i + 1]), BLPoint(lastPosition().x + cmd.fNumbers[i + 2], lastPosition().y + cmd.fNumbers[i + 3]), BLPoint(lastPosition().x + cmd.fNumbers[i + 4], lastPosition().y + cmd.fNumbers[i + 5]));
                }
            }
        }

        // SVG - S, smooth cubicTo

        void smoothCubicTo(const PathSegment& cmd)
        {

            if (cmd.fNumbers.size() < 4) {
                printf("smoothCubicTo - Rejected: %zd\n", cmd.fNumbers.size());
                return;
            }

            fWorkingPath.smoothCubicTo(cmd.fNumbers[0], cmd.fNumbers[1], cmd.fNumbers[2], cmd.fNumbers[3]);

            if (cmd.fNumbers.size() > 4)
            {
                //printf("EXTENDED smoothCubicTo\n");
                for (size_t i = 4; i < cmd.fNumbers.size(); i += 4)
                {
                    fWorkingPath.smoothCubicTo(cmd.fNumbers[i], cmd.fNumbers[i + 1], cmd.fNumbers[i + 2], cmd.fNumbers[i + 3]);
                }
            }

        }

        // SVG - s, smooth cubicBy
        void smoothCubicBy(const PathSegment& cmd)
        {
            //printf("== NYI - smoothCubicBy ==\n");
            if (cmd.fNumbers.size() < 4) {
                printf("smoothCubicBy - Rejected: %zd\n", cmd.fNumbers.size());
                return;
            }

            fWorkingPath.smoothCubicTo(lastPosition().x + cmd.fNumbers[0], lastPosition().y + cmd.fNumbers[1], lastPosition().x + cmd.fNumbers[2], lastPosition().y + cmd.fNumbers[3]);

            if (cmd.fNumbers.size() > 4)
            {
                //printf("EXTENDED smoothCubicBy\n");
                for (size_t i = 4; i < cmd.fNumbers.size(); i += 4)
                {
                    fWorkingPath.smoothCubicTo(lastPosition().x + cmd.fNumbers[i], lastPosition().y + cmd.fNumbers[i + 1], lastPosition().x + cmd.fNumbers[i + 2], lastPosition().y + cmd.fNumbers[i + 3]);
                }
            }
        }

        // SVG - A
        void arcTo(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 7) {
                printf("arcTo - Rejected: %zd\n", cmd.fNumbers.size());
                return;
            }

            double rx = cmd.fNumbers[0];
            double ry = cmd.fNumbers[1];
            double xRotation = cmd.fNumbers[2];
            double largeArcFlag = cmd.fNumbers[3];
            double sweepFlag = cmd.fNumbers[4];
            double x = cmd.fNumbers[5];
            double y = cmd.fNumbers[6];

            bool larc = largeArcFlag > 0.5f;
            bool swp = sweepFlag > 0.5f;
            double rotation = radians(xRotation);

            fWorkingPath.ellipticArcTo(BLPoint(rx, ry), rotation, larc, swp, BLPoint(x, y));

            if (cmd.fNumbers.size() > 7)
            {
                //printf("EXTENDED arcTo\n");
                for (size_t i = 7; i < cmd.fNumbers.size(); i += 7)
                {
                    rx = cmd.fNumbers[i];
                    ry = cmd.fNumbers[i + 1];
                    xRotation = cmd.fNumbers[i + 2];
                    largeArcFlag = cmd.fNumbers[i + 3];
                    sweepFlag = cmd.fNumbers[i + 4];
                    x = cmd.fNumbers[i + 5];
                    y = cmd.fNumbers[i + 6];

                    larc = largeArcFlag > 0.5f;
                    swp = sweepFlag > 0.5f;
                    rotation = radians(xRotation);

                    fWorkingPath.ellipticArcTo(BLPoint(rx, ry), rotation, larc, swp, BLPoint(x, y));
                }
            }
        }


        // SVG - a
        void arcBy(const PathSegment& cmd)
        {
            if (cmd.fNumbers.size() < 7) {
                printf("arcBy - Rejected: %zd\n", cmd.fNumbers.size());
                return;
            }

            float rx = cmd.fNumbers[0];
            float ry = cmd.fNumbers[1];
            float xRotation = cmd.fNumbers[2];
            float largeArcFlag = cmd.fNumbers[3];
            float sweepFlag = cmd.fNumbers[4];
            float x = lastPosition().x + cmd.fNumbers[5];
            float y = lastPosition().y + cmd.fNumbers[6];

            bool larc = largeArcFlag > 0.5f;
            bool swp = sweepFlag > 0.5f;
            float rotation = radians(xRotation);

            fWorkingPath.ellipticArcTo(BLPoint(rx, ry), rotation, larc, swp, BLPoint(x, y));

            if (cmd.fNumbers.size() > 7)
            {
                //printf("EXTENDED arcBy\n");
                for (size_t i = 7; i < cmd.fNumbers.size(); i += 7)
                {
                    rx = cmd.fNumbers[i];
                    ry = cmd.fNumbers[i + 1];
                    xRotation = cmd.fNumbers[i + 2];
                    largeArcFlag = cmd.fNumbers[i + 3];
                    sweepFlag = cmd.fNumbers[i + 4];
                    x = lastPosition().x + cmd.fNumbers[i + 5];
                    y = lastPosition().y + cmd.fNumbers[i + 6];

                    larc = largeArcFlag > 0.5f;
                    swp = sweepFlag > 0.5f;
                    rotation = radians(xRotation);

                    fWorkingPath.ellipticArcTo(BLPoint(rx, ry), rotation, larc, swp, BLPoint(x, y));
                }
            }
        }

        // SVG - Z,z    close path
        void close(const PathSegment& cmd)
        {
            if (!fWorkingPath.empty())
            {
                fWorkingPath.close();
            }
            finishWorking();
        }

        // Turn a set of commands and numbers
        // into a blPath
        //
        void parseCommands(const std::vector<svg2b2d::PathSegment>& segments)
        {
            for (auto& cmd : segments)
            {
                switch (cmd.fCommand)
                {
                case svg2b2d::SegmentKind::MoveTo:
                    moveTo(cmd);
                    break;
                case svg2b2d::SegmentKind::MoveBy:
                    moveBy(cmd);
                    break;

                case svg2b2d::SegmentKind::LineTo:
                    lineTo(cmd);
                    break;
                case svg2b2d::SegmentKind::LineBy:
                    lineBy(cmd);
                    break;

                case svg2b2d::SegmentKind::HLineTo:
                    hLineTo(cmd);
                    break;
                case svg2b2d::SegmentKind::HLineBy:
                    hLineBy(cmd);
                    break;

                case svg2b2d::SegmentKind::VLineTo:
                    vLineTo(cmd);
                    break;
                case svg2b2d::SegmentKind::VLineBy:
                    vLineBy(cmd);
                    break;

                case svg2b2d::SegmentKind::CubicTo:
                    cubicTo(cmd);
                    break;
                case svg2b2d::SegmentKind::CubicBy:
                    cubicBy(cmd);
                    break;

                case svg2b2d::SegmentKind::SCubicTo:
                    smoothCubicTo(cmd);
                    break;
                case svg2b2d::SegmentKind::SCubicBy:
                    smoothCubicBy(cmd);
                    break;

                case svg2b2d::SegmentKind::QuadTo:
                    quadTo(cmd);
                    break;
                case svg2b2d::SegmentKind::QuadBy:
                    quadBy(cmd);
                    break;

                case SegmentKind::SQuadTo:
                    smoothQuadTo(cmd);
                    break;
                case SegmentKind::SQuadBy:
                    smoothQuadBy(cmd);
                    break;


                case svg2b2d::SegmentKind::ArcTo:
                    arcTo(cmd);
                    break;
                case svg2b2d::SegmentKind::ArcBy:
                    arcBy(cmd);
                    break;


                case svg2b2d::SegmentKind::CloseTo:
                case svg2b2d::SegmentKind::CloseBy:
                    close(cmd);
                    break;

                default:
                    // do nothing
                    break;
                }

                //lastPosition();
                fLastCommand = cmd.fCommand;
            }

            finishWorking();

            return;
        }


    };

    static bool blPathFromSegments(std::vector<svg2b2d::PathSegment>& segments, BLPath& apath)
    {
        PathBuilder builder{};

        builder.parseCommands(segments);
        apath = builder.getPath();

        return true;
    }

    static bool blPathFromCommands(ByteSpan& chunk, BLPath& apath)
    {
        std::vector<svg2b2d::PathSegment> segments{};

        svg2b2d::tokenizePath(chunk, segments);

        return blPathFromSegments(segments, apath);
    }
}


namespace svg2b2d {
    // A map between a name and a presentation attribute kind
    // Functions can use this mapping to determine how to parse the data
    // https://www.w3.org/TR/SVG2/attindex.html#PresentationAttributes
    // 
    // It's questionable whether this mapping is needed.  
    // It might be useful when we're doing generic parsing of attributes
    // but not so useful when we're doing highly directed parsing, where we
    // explicitly already know the types we're parsing.
    // 
    // It might be useful to help the programmer to determine which basic
    // type parser to use.  So, we'll keep it as informational for now
    std::map<std::string, int> SVGPresentationAttributeMap = {
        {"alignment-baseline", SVG_ATTR_KIND_CHUNK}
        ,{"baseline-shift", SVG_ATTR_KIND_CHUNK}
        ,{"clip", SVG_ATTR_KIND_ENUM}
        ,{"clip-path", SVG_ATTR_KIND_CHUNK}
        ,{"clip-rule", SVG_ATTR_KIND_CHUNK}
        ,{"color", SVG_ATTR_KIND_CHUNK}
        ,{"color-interpolation", SVG_ATTR_KIND_CHUNK}
        ,{"color-interpolation-filters", SVG_ATTR_KIND_CHUNK}
        ,{"color-rendering", SVG_ATTR_KIND_CHUNK}
        ,{"cursor", SVG_ATTR_KIND_CHUNK}
        ,{"direction", SVG_ATTR_KIND_CHUNK}
        ,{"display", SVG_ATTR_KIND_CHUNK}
        ,{"dominant-baseline", SVG_ATTR_KIND_CHUNK}
        ,{"fill", SVG_ATTR_KIND_PAINT}
        ,{"fill-opacity", SVG_ATTR_KIND_NUMBERORPERCENT}
        ,{"fill-rule", SVG_ATTR_KIND_ENUM}
        ,{"filter", SVG_ATTR_KIND_CHUNK}
        ,{"flood-color", SVG_ATTR_KIND_CHUNK}
        ,{"flood-opacity", SVG_ATTR_KIND_NUMBERORPERCENT}
        ,{"font-family", SVG_ATTR_KIND_CHUNK}
        ,{"font-size", SVG_ATTR_KIND_DIMENSION}
        ,{"font-size-adjust", SVG_ATTR_KIND_CHUNK}
        ,{"font-stretch", SVG_ATTR_KIND_CHUNK}
        ,{"font-style", SVG_ATTR_KIND_CHUNK}
        ,{"font-variant", SVG_ATTR_KIND_CHUNK}
        ,{"font-weight", SVG_ATTR_KIND_CHUNK}
        ,{"glyph-orientation-horizontal", SVG_ATTR_KIND_CHUNK}
        ,{"glyph-orientation-vertical", SVG_ATTR_KIND_CHUNK}
        ,{"image-rendering", SVG_ATTR_KIND_CHUNK}
        ,{"lighting-color", SVG_ATTR_KIND_COLOR}
        ,{"marker-end", SVG_ATTR_KIND_CHUNK}
        ,{"marker-mid", SVG_ATTR_KIND_CHUNK}
        ,{"marker-start", SVG_ATTR_KIND_CHUNK}
        ,{"mask", SVG_ATTR_KIND_CHUNK}
        ,{"opacity", SVG_ATTR_KIND_NUMBERORPERCENT}
        ,{"overflow", SVG_ATTR_KIND_CHUNK}
        ,{"paint-order", SVG_ATTR_KIND_ENUM}                        // normal | [fill || stroke || markers]
        ,{"pointer-events", SVG_ATTR_KIND_CHUNK}
        ,{"shape-rendering", SVG_ATTR_KIND_CHUNK}
        ,{"stop-color", SVG_ATTR_KIND_CHUNK}
        ,{"stop-opacity", SVG_ATTR_KIND_NUMBERORPERCENT}
        ,{"stroke", SVG_ATTR_KIND_PAINT}
        ,{"stroke-dasharray", SVG_ATTR_KIND_CHUNK}
        ,{"stroke-dashoffset", SVG_ATTR_KIND_DIMENSION}
        ,{"stroke-linecap", SVG_ATTR_KIND_ENUM}                     // butt, round, square
        ,{"stroke-linejoin", SVG_ATTR_KIND_ENUM}                    // miter, miter-clip, round, bevel, arcs
        ,{"stroke-miterlimit", SVG_ATTR_KIND_NUMBER}
        ,{"stroke-opacity", SVG_ATTR_KIND_NUMBERORPERCENT}
        ,{"stroke-width", SVG_ATTR_KIND_DIMENSION}
        ,{"text-anchor", SVG_ATTR_KIND_ENUM}                       // start, middle, end
        ,{"text-decoration", SVG_ATTR_KIND_CHUNK}
        ,{"text-rendering", SVG_ATTR_KIND_CHUNK}
        ,{"transform", SVG_ATTR_KIND_TRANSFORM}
        ,{"unicode-bidi", SVG_ATTR_KIND_CHUNK}
        ,{"vector-effect", SVG_ATTR_KIND_CHUNK}
        ,{"vertical-align", SVG_ATTR_KIND_DIMENSION}               // SVG 2.0
        ,{"visibility", SVG_ATTR_KIND_CHUNK}
        ,{"word-spacing", SVG_ATTR_KIND_DIMENSION}
        ,{"letter-spacing", SVG_ATTR_KIND_CHUNK}
    };
}

namespace svg2b2d {
    struct IMapSVGNodes;
    
    struct SVGObject : public IDrawable
    {
        IMapSVGNodes* fRoot{ nullptr };
        std::string fName{};    // The tag name of the element
        BLVar fVar{};
        bool fIsVisible{ false };
        BLBox fExtent{};

        
		SVGObject() = delete;
        SVGObject(const SVGObject& other) :fName(other.fName) {}
        SVGObject(IMapSVGNodes* root) :fRoot(root) {}
		virtual ~SVGObject() = default;
        
		SVGObject& operator=(const SVGObject& other) {
            fRoot = other.fRoot;
            fName = other.fName;
			BLVar fVar = other.fVar;
            
            return *this;
		}
        
		IMapSVGNodes* root() const { return fRoot; }
        virtual void setRoot(IMapSVGNodes* root) { fRoot = root; }
        
        const std::string& name() const { return fName; }
        void setName(const std::string& name) { fName = name; }

		const bool visible() const { return fIsVisible; }
		void setVisible(bool visible) { fIsVisible = visible; }
        
        
        // sub-classes should return something interesting as BLVar
        // This can be used for styling, so images, colors, patterns, gradients, etc
        virtual const BLVar& getVariant()
        {
            return fVar;
        }
        

        
        void draw(BLContext& ctx) override
        {
            ;// draw the object
        }
        
        virtual void loadSelfFromXml(const XmlElement& elem)
        {
            ;
        }

        virtual void loadFromXmlElement(const svg2b2d::XmlElement& elem)
        {
            // load the common attributes
            setName(elem.name());

            // call to loadselffromxml
            // so sub-class can do its own loading
            loadSelfFromXml(elem);
        }
    };
    
    struct IMapSVGNodes
    {
        virtual std::shared_ptr<SVGObject> findNodeById(const std::string& name) = 0;
        virtual std::shared_ptr<SVGObject> findNodeByHref(const ByteSpan& href) = 0;

        
        virtual void addDefinition(const std::string& name, std::shared_ptr<SVGObject> obj) = 0;

        virtual void setInDefinitions(bool indefs) = 0;
        virtual bool inDefinitions() const = 0;
    };
    
    /*
    struct SVGVisual : public SVGObject
    {
        SVGVisual(): SVGObject(){}
		SVGVisual(IMapSVGNodes* root) : SVGObject(root) {}
        SVGVisual(const SVGVisual& other) :SVGObject(other) {}
        
		virtual ~SVGVisual() = default;

		SVGVisual& operator=(const SVGVisual& other)
		{
			SVGObject::operator=(other);
			return *this;
		}
    };
    */
    
    
    // SVGVisualProperty
    // This is meant to be the base class for things that are optionally
    // used to alter the graphics context.
    // If isSet() is true, then the drawSelf() is called.
	// sub-classes should override drawSelf() to do the actual drawing
    //
    // This is used for things like; Paint, Transform, Miter, etc.
    //
    struct SVGVisualProperty :  public SVGObject
    {
        bool fIsSet{ false };

        //SVGVisualProperty() :SVGObject(),fIsSet(false){}
        SVGVisualProperty(IMapSVGNodes *root):SVGObject(root),fIsSet(false){}
        SVGVisualProperty(const SVGVisualProperty& other)
            :SVGObject(other)
            ,fIsSet(other.fIsSet)
        {}

        SVGVisualProperty operator=(const SVGVisualProperty& rhs)
        {
            SVGObject::operator=(rhs);
            fIsSet = rhs.fIsSet;
            
            return *this;
        }

        void set(const bool value) { fIsSet = value; }
        bool isSet() const { return fIsSet; }

		virtual void loadSelfFromChunk(const ByteSpan& chunk)
        {
			;
        }

        virtual void loadFromChunk(const ByteSpan& chunk)
        {
			loadSelfFromChunk(chunk);
        }
        
        // Apply propert to the context conditionally
        virtual void drawSelf(BLContext& ctx)
        {
            ;
        }

        void draw(BLContext& ctx) override
        {
            if (isSet())
                drawSelf(ctx);
        }

    };
}



// Specific types of attributes
//==============================================================================
// SVGDimension
// used for length, time, frequency, resolution, location
//==============================================================================
namespace svg2b2d
{
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
        float fValue;
        SVGDimensionUnits fUnits;
        
        float fDeviceValue;
        
        SVGDimension()
			: fValue(0.0f)
			, fUnits(SVGDimensionUnits::SVG_UNITS_USER)
			, fDeviceValue(0.0f)
		{
		}
	    
        SVGDimension(const SVGDimension &other)
            :fValue(other.fValue)
            ,fUnits(other.fUnits)
            ,fDeviceValue(other.fDeviceValue)
        {}

        SVGDimension(float value, SVGDimensionUnits units)
		: fValue(value)
		, fUnits(units)
		, fDeviceValue(0.0f)
	    {
	    }

        float value() const { return fValue; }
        SVGDimensionUnits units() const { return fUnits; }
        float calculatePixels(float length=1.0, float orig=0, float dpi = 96)
        {
            //static float svg_convertToPixels(float orig, float length)


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
        
        SVGDimension& reset(float value, SVGDimensionUnits units)
        {
			fValue = value;
			fUnits = units;
			fDeviceValue = 0.0f;

			return *this;
        }
        
        void loadSelfFromChunk(const ByteSpan &inChunk)
		{
            ByteSpan s = inChunk;
            ByteSpan numChunk;
            auto nextPart = scanNumber(s, numChunk);
            float value = (float)chunk_to_double(numChunk);
            SVGDimensionUnits units = parseDimensionUnits(nextPart);
            
            reset(value, units);
		}
    };


    
    static inline SVGDimension toDimension(const ByteSpan& inChunk)
    {
        SVGDimension dim{};
        dim.loadSelfFromChunk(inChunk);

        return dim;
    }

    
    struct SVGPoint {
        float fX;
        float fY;

        SVGPoint(float x, float y) : fX(x), fY(y) {}
        SVGPoint() : fX(0), fY(0) {}

        float x() const { return fX; }
        float y() const { return fY; }

        static SVGPoint fromChunk(const ByteSpan& inChunk)
        {
            SVGPoint point{};

            ByteSpan s = inChunk;
            ByteSpan numChunk;
            charset numDelims = wspChars + ',';

            point.fX = (float)nextNumber(s, numDelims);
            point.fY = (float)nextNumber(s, numDelims);

            return point;
        }


    };
}


namespace svg2b2d {
    struct SVGOpacity : public SVGVisualProperty
    {
        float fValue{ 1.0 };

		SVGOpacity(IMapSVGNodes* iMap):SVGVisualProperty(iMap){}


        void drawSelf(BLContext& ctx)
        {
			SVGVisualProperty::drawSelf(ctx);
			ctx.setFillAlpha(fValue);
        }

		void loadSelfFromChunk(const ByteSpan& inChunk)
		{
            fValue = toDimension(inChunk).calculatePixels(1);
            set(true);
		}

        static std::shared_ptr<SVGOpacity> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
            std::shared_ptr<SVGOpacity> sw = std::make_shared<SVGOpacity>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                sw->loadFromChunk(inChunk);

            return sw;
        }

        static std::shared_ptr<SVGOpacity> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            return createFromChunk(root, name, elem.getAttribute(name));
        }
	};

}


namespace svg2b2d {
    struct SVGFontSize : public SVGVisualProperty
    {
        double fValue{ 12.0 };

        //SVGFontSize() : SVGVisualProperty() {}
		SVGFontSize(IMapSVGNodes* inMap) : SVGVisualProperty(inMap) {}
        SVGFontSize(const SVGFontSize& other) :SVGVisualProperty(other) { fValue = other.fValue; }
        
        SVGFontSize& operator=(const SVGFontSize& rhs)
        {
            SVGVisualProperty::operator=(rhs);
            fValue = rhs.fValue;
            return *this;
        }

        void drawSelf(BLContext& ctx)
        {
            //ctx.textSize(fValue);
        }

        void loadSelfFromChunk(const ByteSpan& inChunk)
        {
            fValue = toDimension(inChunk).calculatePixels(96);
            set(true);
        }

        static std::shared_ptr<SVGFontSize> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
            std::shared_ptr<SVGFontSize> sw = std::make_shared<SVGFontSize>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                sw->loadFromChunk(inChunk);

            return sw;
        }

        static std::shared_ptr<SVGFontSize> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            return createFromChunk(root, name, elem.getAttribute(name));
        }
    };


    
    // Text Alignment
enum class ALIGNMENT : unsigned
{
    MIDDLE = 0x01,
    START = 0x02,
    END = 0x04,
} ;
    
    struct SVGTextAnchor : public SVGVisualProperty
    {
        ALIGNMENT fValue{ ALIGNMENT::START };
        
        //SVGTextAnchor() : SVGVisualProperty() {}
		SVGTextAnchor(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
        SVGTextAnchor(const SVGTextAnchor& other) :SVGVisualProperty(other) { fValue = other.fValue; }
        
        SVGTextAnchor& operator=(const SVGTextAnchor& rhs)
        {
            SVGVisualProperty::operator=(rhs);
            fValue = rhs.fValue;
            return *this;
        }

        void drawSelf(BLContext& ctx)
        {
            // BUGBUG, need to calculate alignment
            //ctx.textAlign(fValue, ALIGNMENT::BASELINE);
        }

        void loadSelfFromChunk(const ByteSpan& inChunk)
        {
            if (inChunk == "start")
                fValue = ALIGNMENT::START;
			else if (inChunk == "middle")
				fValue = ALIGNMENT::MIDDLE;
			else if (inChunk == "end")
				fValue = ALIGNMENT::END;


            set(true);
        }

        static std::shared_ptr<SVGTextAnchor> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
            std::shared_ptr<SVGTextAnchor> sw = std::make_shared<SVGTextAnchor>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                sw->loadFromChunk(inChunk);

            return sw;
        }

        static std::shared_ptr<SVGTextAnchor> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            return createFromChunk(root, name, elem.getAttribute(name));
        }
    };
    
    struct SVGTextAlign : public SVGVisualProperty
    {
        ALIGNMENT fValue{ ALIGNMENT::START };

		SVGTextAlign(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
        SVGTextAlign(const SVGTextAlign& other) :SVGVisualProperty(other) { fValue = other.fValue; }
        
        SVGTextAlign& operator=(const SVGTextAlign& rhs)
        {
            SVGVisualProperty::operator=(rhs);
            fValue = rhs.fValue;
            return *this;
        }

        void drawSelf(BLContext& ctx)
        {
			// BUGBUG, need to calculate alignment
            //ctx.textAlign(fValue, ALIGNMENT::BASELINE);
        }

        void loadSelfFromChunk(const ByteSpan& inChunk)
        {
            if (inChunk == "center")
				fValue = ALIGNMENT::MIDDLE;
            
            set(true);
        }

        static std::shared_ptr<SVGTextAlign> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
            std::shared_ptr<SVGTextAlign> sw = std::make_shared<SVGTextAlign>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                sw->loadFromChunk(inChunk);

            return sw;
        }

        static std::shared_ptr<SVGTextAlign> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            return createFromChunk(root, name, elem.getAttribute(name));
        }
    };
}


namespace svg2b2d {
    void parseStyleAttribute(const ByteSpan & inChunk, XmlElement &styleElement)
    {
        // Turn the style element into attributes of an XmlElement, 
        // then, the caller can use that to more easily parse whatever they're
        // looking for.
        ByteSpan styleChunk = inChunk;
        
        if (styleChunk) {
            // use CSSInlineIterator to iterate through the key value pairs
            // creating a visual attribute, using the gSVGPropertyCreation map
            CSSInlineStyleIterator iter(styleChunk);

            while (iter.next())
            {
                std::string name = std::string((*iter).first.fStart, (*iter).first.fEnd);
                if (!name.empty() && (*iter).second)
                {
                    styleElement.addAttribute(name, (*iter).second);
                }
            }

        }

        return;
    }
}

//======================================================
// Definition of SVG Paint
//======================================================
namespace svg2b2d {
    // Representation of color according to CSS specification
	// https://www.w3.org/TR/css-color-4/#typedef-color
    // Over time, this structure could represent the full specification
    // but for practical purposes, we'll focus on rgb, rgba for now
    /*
    <color> = <absolute-color-base> | currentcolor | <system-color> 

    <absolute-color-base> = <hex-color> | <absolute-color-function> | <named-color> | transparent
    <absolute-color-function> = <rgb()> | <rgba()> |
                            <hsl()> | <hsla()> | <hwb()> |
                            <lab()> | <lch()> | <oklab()> | <oklch()> |
                            <color()>
    */

}

namespace svg2b2d {

    static BLRgba32 parseColorHex(const ByteSpan& chunk)
    {
        // BUGBUG - making a copy of the chunk to use sscanf
        char str[64];
        copy_to_cstr(str, 63, chunk);

        unsigned int r = 0, g = 0, b = 0;
        if (sscanf_s(str, "#%2x%2x%2x", &r, &g, &b) == 3)		// 2 digit hex
            return BLRgba32(r, g, b);
        if (sscanf_s(str, "#%1x%1x%1x", &r, &g, &b) == 3)		// 1 digit hex, e.g. #abc -> 0xccbbaa
            return BLRgba32(r * 17, g * 17, b * 17);			    // same effect as (r<<4|r), (g<<4|g), ..

        // if not one of those cases, just return a mid-gray
        return BLRgba32(128, 128, 128);
    }



    // Parse rgb color. The pointer 'str' must point at "rgb(" (4+ characters).
    // This function returns gray (rgb(128, 128, 128) == '#808080') on parse errors
    // for backwards compatibility. Note: other image viewers return black instead.

    static BLRgba32 parseColorRGB(const ByteSpan& inChunk)
    {
        // skip past the leading "rgb("
        ByteSpan s = inChunk;
        auto leading = chunk_token(s, "(");

        // s should now point to the first number
        // and 'leading' should contain 'rgb'
        // BUGBUG - we can check 'leading' to see if it's actually 'rgb'
        // but we'll just assume it is for now

        // get the numbers by separating at the ')'
        auto nums = chunk_token(s, ")");

        // So, now nums contains the individual numeric values, separated by ','
        // The individual numeric values are either
        // 50%
        // 50

        int i = 0;
        uint8_t rgbi[4]{};

        // Get the first token, which is red
        // if it's not there, then return gray
        auto num = chunk_token(nums, ",");
        if (chunk_size(num) < 1)
            return BLRgba32(128, 128, 128, 0);

        while (num)
        {
            auto cv = toDimension(num);

            //writeChunk(num);
            if (chunk_find_char(num, '%'))
            {
                // it's a percentage
                // BUGBUG - we're assuming it's a range of [0..255]

                rgbi[i] = (uint8_t)(cv.value() / 100.0f * 255.0f);
            }
            else
            {
                // it's a regular number
                if (i==3)
                    rgbi[i] = (uint8_t)(cv.value()  * 255.0f);
                else 
                    rgbi[i] = (uint8_t)cv.value();
            }
            i++;
            num = chunk_token(nums, ",");
        }

        if (i == 4)
            return BLRgba32(rgbi[0], rgbi[1], rgbi[2], rgbi[3]);

        return BLRgba32(rgbi[0], rgbi[1], rgbi[2]);
    }

    static BLRgba32 parseColorName(const ByteSpan& inChunk)
    {
        std::string cName = std::string(inChunk.fStart, inChunk.fEnd);

        // If named color not found
        // or name == "none"
        // return fully transparent black
        // BUGBUG - this is different than not having a color attribute
        // in which case, we might want to eliminate color, and allow ancestor's color to come through
        if (!svg::colors.contains(cName))
            return BLRgba32(128, 128, 128, 255);

        return svg::colors[cName];
    }



    constexpr int SVG_PaintForFill = 1;
    constexpr int SVG_PaintForStroke = 2;

    struct SVGPaint : public SVGVisualProperty
    {
        BLVar fPaint{};
        bool fExplicitNone{ false };
        int fPaintFor{ SVG_PaintForFill };

		SVGPaint(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
        SVGPaint(const SVGPaint& other) :SVGVisualProperty(other) 
        {

        }

        SVGPaint& operator=(const SVGPaint& rhs)
        {
            SVGVisualProperty::operator=(rhs);

            blVarAssignWeak(&fPaint, &rhs.fPaint);
            fExplicitNone = rhs.fExplicitNone;
            fPaintFor = rhs.fPaintFor;

            return *this;
        }

        const BLVar& getVariant() override
        {
            return fPaint;
        }
        
        void setPaintFor(int pfor) { fPaintFor = pfor; }
        void setOpacity(float opacity)
        {
            uint32_t outValue;
            if (BL_SUCCESS == blVarToRgba32(&fPaint, &outValue))
            {
                BLRgba32 newColor(outValue);
                newColor.setA(opacity * 255);
                blVarAssignRgba32(&fPaint, newColor.value);
            }
        }

        // Need to distinguish which function gets called
        // BUGBUG
        void drawSelf(BLContext& ctx)
        {
            switch (fPaintFor)
            {

            case SVG_PaintForFill:
                if (fExplicitNone)
                    ctx.setFillStyle(BLRgba32(0));
                else
                    ctx.setFillStyle(fPaint);
                break;

            case SVG_PaintForStroke:
                if (fExplicitNone)
					ctx.setStrokeStyle(BLRgba32(0));
                else
                    ctx.setStrokeStyle(fPaint);
                break;
            }

        }

        void loadFromUrl(const ByteSpan &inChunk)
        {
            ByteSpan str = inChunk;
            
            // the id we want should look like this
            // url(#id)
            // so we need to skip past the 'url(#'
            // and then find the closing ')'
            // and then we have the id
            auto url = chunk_token(str, "(");
            auto id = chunk_trim(chunk_token(str, ")"), wspChars);

            // The first character could be '.' or '#'
            // so we need to skip past that
            if (*id == '.' || *id == '#')
                id++;

            if (!id)
                return;

            // lookup the thing we're referencing
            std::string idStr = toString(id);

            if (fRoot != nullptr)
            {
                auto node = fRoot->findNodeById(idStr);
                
                // pull out the color value
                if (node != nullptr)
                {
                    const BLVar& aVar = node->getVariant();

                    auto res = blVarAssignWeak(&fPaint, &aVar);
                    if (res == BL_SUCCESS)
                        set(true);
                }
            }
        }
        
        void loadSelfFromChunk (const ByteSpan& inChunk)
        {
            BLRgba32 c{};
            
            blVarAssignRgba32(&fPaint, c.value);

            ByteSpan str = inChunk;
            ByteSpan rgbStr = chunk_from_cstr("rgb(");
            ByteSpan rgbaStr = chunk_from_cstr("rgba(");

            size_t len = 0;

            len = chunk_size(str);
            if (len >= 1 && *str == '#')
            {
                c = parseColorHex(str);
                blVarAssignRgba32(&fPaint, c.value);
                set(true);
            }
            else if (chunk_starts_with(str, rgbStr) || chunk_starts_with(str, rgbaStr))
            {
                c = parseColorRGB(str);
                blVarAssignRgba32(&fPaint, c.value);
                set(true);
            }
            else if (chunk_starts_with_cstr(str, "url(")) 
            {
                loadFromUrl(str);
            }
            else {
                std::string cName = std::string(str.fStart, str.fEnd);
                if (cName == "none") {
                    fExplicitNone = true;
                    set(true);
                }
                else if (svg::colors.contains(cName))
                {
                    c = svg::colors[cName];
                    blVarAssignRgba32(&fPaint, c.value);
                    set(true);
                }
                else {
                    // user wants some sort of color, which is either invalid name
                    // or a color function we don't support yet
                    // so set a default gray color
                    c = BLRgba32(128, 128, 128);
                    blVarAssignRgba32(&fPaint, c.value);
                    set(true);
                }
            }


        }

        static std::shared_ptr<SVGPaint> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
            std::shared_ptr<SVGPaint> paint = std::make_shared<SVGPaint>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                paint->loadFromChunk(inChunk);

            return paint;
        }

        static std::shared_ptr<SVGPaint> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            auto paint = createFromChunk(root, name, elem.getAttribute(name));

			if (!paint->isSet())
				return paint;
            
            if (name == "fill")
            {
                paint->setPaintFor(SVG_PaintForFill);
                // look for fill-opacity as well
                auto o = elem.getAttribute("fill-opacity");
                if (o)
                {
                    auto onum = toNumber(o);
                    paint->setOpacity(onum);
                }
            }
            else if (name == "stroke")
            {
                paint->setPaintFor(SVG_PaintForStroke);
                // look for stroke-opacity as well
                auto o = elem.getAttribute("stroke-opacity");
                if (o)
                {
                    auto onum = toNumber(o);
                    paint->setOpacity(onum);
                }
            }
			else if (name == "stop-color")
			{
				//paint->setPaintFor(SVG_PaintForStopColor);
				// look for stop-opacity as well
				auto o = elem.getAttribute("stop-opacity");
				if (o)
				{
					auto onum = toNumber(o);
					paint->setOpacity(onum);
				}
			}


            return paint;
        }
    };
}

//enum class FILLRULE : unsigned
//{
//    NON_ZERO = 0,
//    EVEN_ODD = 1,
//};
namespace svg2b2d {
    struct SVGFillRule : public SVGVisualProperty
    {
		BLFillRule fValue{ BL_FILL_RULE_NON_ZERO };

   
        //SVGFillRule() : SVGVisualProperty() {}
		SVGFillRule(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
        SVGFillRule(const SVGFillRule& other) :SVGVisualProperty(other)
        {
            fValue = other.fValue;
        }

        SVGFillRule& operator=(const SVGFillRule& rhs)
        {
            SVGVisualProperty::operator=(rhs);
            fValue = rhs.fValue;
            return *this;
        }

        void drawSelf(BLContext& ctx)
        {
			ctx.setFillRule(fValue);
        }

        void loadSelfFromChunk(const ByteSpan& inChunk)
        {
            ByteSpan s = chunk_trim(inChunk, wspChars);

            set(true);

            if (s == "nonzero")
                fValue = BL_FILL_RULE_NON_ZERO;
            else if (s == "evenodd")
                fValue = BL_FILL_RULE_EVEN_ODD;
            else
                set(false);
        }

        static std::shared_ptr<SVGFillRule> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
            std::shared_ptr<SVGFillRule> node = std::make_shared<SVGFillRule>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                node->loadFromChunk(inChunk);

            return node;
        }

        static std::shared_ptr<SVGFillRule> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            return createFromChunk(root, name, elem.getAttribute(name));
        }
    };
}

namespace svg2b2d {
    struct SVGStrokeWidth : public SVGVisualProperty
    {
		double fWidth{ 1.0};

		//SVGStrokeWidth() : SVGVisualProperty() {}
		SVGStrokeWidth(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
		SVGStrokeWidth(const SVGStrokeWidth& other) :SVGVisualProperty(other) { fWidth = other.fWidth; }
        
		SVGStrokeWidth& operator=(const SVGStrokeWidth& rhs)
		{
			SVGVisualProperty::operator=(rhs);
			fWidth = rhs.fWidth;
			return *this;
		}

		void drawSelf(BLContext& ctx)
		{
			ctx.setStrokeWidth(fWidth);
		}

		void loadSelfFromChunk(const ByteSpan& inChunk)
		{
			fWidth = toNumber(inChunk);
			set(true);
		}

		static std::shared_ptr<SVGStrokeWidth> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
		{
			std::shared_ptr<SVGStrokeWidth> sw = std::make_shared<SVGStrokeWidth>(root);

			// If the chunk is empty, return immediately 
			if (inChunk)
				sw->loadFromChunk(inChunk);

			return sw;
		}

		static std::shared_ptr<SVGStrokeWidth> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
		{
			return createFromChunk(root, name, elem.getAttribute(name));
		}
    };
    
    /// <summary>
    ///  SVGStrokeMiterLimit
	/// A visual property to set the miter limit for a stroke
    /// </summary>
    struct SVGStrokeMiterLimit : public SVGVisualProperty
    {
		double fMiterLimit{ 4.0 };
        
        //SVGStrokeMiterLimit() : SVGVisualProperty() {}
		SVGStrokeMiterLimit(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
		SVGStrokeMiterLimit(const SVGStrokeMiterLimit& other) :SVGVisualProperty(other) { fMiterLimit = other.fMiterLimit; }
        
        SVGStrokeMiterLimit& operator=(const SVGStrokeMiterLimit& rhs)
		{
			SVGVisualProperty::operator=(rhs);
			fMiterLimit = rhs.fMiterLimit;
			return *this;
        }

		void drawSelf(BLContext& ctx)
		{
			ctx.setStrokeMiterLimit(fMiterLimit);
		}
        
		void loadSelfFromChunk(const ByteSpan& inChunk)
		{
			fMiterLimit = toNumber(inChunk);
			fMiterLimit = clamp((float)fMiterLimit, 1.0f, 10.0f);
            
			set(true);
		}

		static std::shared_ptr<SVGStrokeMiterLimit> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
		{
			std::shared_ptr<SVGStrokeMiterLimit> sw = std::make_shared<SVGStrokeMiterLimit>(root);

			// If the chunk is empty, return immediately 
			if (inChunk)
				sw->loadFromChunk(inChunk);

			return sw;
		}

        // stroke-miterlimit
		static std::shared_ptr<SVGStrokeMiterLimit> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
		{
			return createFromChunk(root, name, elem.getAttribute(name));
		}
	
    };
    
    struct SVGStrokeLineCap : public SVGVisualProperty
    {
        BLStrokeCap fLineCap{ BL_STROKE_CAP_BUTT };


		SVGStrokeLineCap(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
		SVGStrokeLineCap(const SVGStrokeLineCap& other) :SVGVisualProperty(other)
		{
			fLineCap = other.fLineCap;
		}

		SVGStrokeLineCap& operator=(const SVGStrokeLineCap& rhs)
		{
			SVGVisualProperty::operator=(rhs);
			fLineCap = rhs.fLineCap;
			return *this;
		}

		void drawSelf(BLContext& ctx)
		{
            ctx.setStrokeCaps(fLineCap);
		}

		void loadSelfFromChunk(const ByteSpan& inChunk)
		{
            ByteSpan s = chunk_trim(inChunk, wspChars);

            set(true);
            
			if (s == "butt")
				fLineCap = BL_STROKE_CAP_BUTT;
			else if (s == "round")
				fLineCap = BL_STROKE_CAP_ROUND;
			else if (s == "square")
				fLineCap = BL_STROKE_CAP_SQUARE;
            else
			    set(false);
		}

		static std::shared_ptr<SVGStrokeLineCap> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
		{
			std::shared_ptr<SVGStrokeLineCap> stroke = std::make_shared<SVGStrokeLineCap>(root);

			// If the chunk is empty, return immediately 
			if (inChunk)
				stroke->loadFromChunk(inChunk);

			return stroke;
		}

		static std::shared_ptr<SVGStrokeLineCap> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem )
		{
			return createFromChunk(root, name, elem.getAttribute(name));
		}
	};

    // SVGStrokeLineJoin
	// A visual property to set the line join for a stroke
    struct SVGStrokeLineJoin : public SVGVisualProperty
    {
        BLStrokeJoin fLineJoin{ SVG_JOIN_MITER_BEVEL };

		SVGStrokeLineJoin(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
		SVGStrokeLineJoin(const SVGStrokeLineJoin& other) :SVGVisualProperty(other), fLineJoin(other.fLineJoin) {}  
        
        SVGStrokeLineJoin& operator=(const SVGStrokeLineJoin& rhs)
        {
			SVGVisualProperty::operator=(rhs);
			fLineJoin = rhs.fLineJoin;
			return *this;
        }
        
        void drawSelf(BLContext& ctx)
        {
			ctx.setStrokeJoin(fLineJoin);
        }
        
        void loadSelfFromChunk(const ByteSpan& inChunk)
        {
            ByteSpan s = chunk_trim(inChunk, wspChars);

            set(true);
            
			if (s == "miter")
				fLineJoin = BL_STROKE_JOIN_MITER_BEVEL;
			else if (s == "round")
				fLineJoin = BL_STROKE_JOIN_ROUND;
			else if (s == "bevel")
				fLineJoin = BL_STROKE_JOIN_BEVEL;
            //else if (s == "arcs")
			//	fLineJoin = SVG_JOIN_ARCS;
			else if (s == "miter-clip")
				fLineJoin = BL_STROKE_JOIN_MITER_CLIP;
			else
				set(false);

        }
        
		static std::shared_ptr<SVGStrokeLineJoin> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
		{
			std::shared_ptr<SVGStrokeLineJoin> stroke = std::make_shared<SVGStrokeLineJoin>(root);

			// If the chunk is empty, return immediately
			if (inChunk)
				stroke->loadFromChunk(inChunk);

			return stroke;
		}

        // stroke-linejoin
        static std::shared_ptr<SVGStrokeLineJoin> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
			return createFromChunk(root, name, elem.getAttribute(name));
        }
        
    };
    
}
namespace svg2b2d {
    
    //======================================================
    // SVGViewbox
    // A document may or may not have this property
    //======================================================
    
    struct SVGViewbox : public SVGVisualProperty
    {
        BLRect fRect{};


        SVGViewbox() :SVGVisualProperty(nullptr) {}
        //SVGViewbox():SVGVisualProperty(){}
		SVGViewbox(IMapSVGNodes* iMap):SVGVisualProperty(iMap){}
        SVGViewbox(const SVGViewbox& other)
            : SVGVisualProperty(other)
            ,fRect(other.fRect)
        {

        }

        SVGViewbox& operator=(const SVGViewbox& rhs)
        {
            SVGVisualProperty::operator=(rhs);
            fRect = rhs.fRect;

            return *this;
        }

        float x() const { return fRect.x; }
        float y() const { return fRect.y; }
        float width() const { return fRect.w; }
        float height() const {return fRect.h;}
        
        void loadSelfFromChunk(const ByteSpan& inChunk)
        {
			ByteSpan s = inChunk;
			ByteSpan numChunk;
            charset numDelims = wspChars+',';
                
            fRect.x = nextNumber(s, numDelims);
			fRect.y = nextNumber(s, numDelims);
			fRect.w = nextNumber(s, numDelims);
			fRect.h = nextNumber(s, numDelims);
        }

        static SVGViewbox createFromChunk(IMapSVGNodes* root, const ByteSpan& inChunk)
        {
            SVGViewbox vbox{};

            // If the chunk is empty, return immediately 
            if (!inChunk)
                return vbox;

            vbox.loadFromChunk(inChunk);

            return vbox;
        }

        // "viewBox"
        static SVGViewbox createFromXml(IMapSVGNodes* root, const XmlElement& elem, const std::string &name)
        {
            return createFromChunk(root, elem.getAttribute(name));
        }
    };
    


}

namespace svg2b2d
{


    static std::vector<SVGPoint> toPoints(const ByteSpan &inChunk)
	{
		std::vector<SVGPoint> points;

		ByteSpan s = inChunk;
		charset numDelims = wspChars + ',';

		while (s)
		{
			SVGPoint p;
			p.fX = nextNumber(s, numDelims);
			p.fY = nextNumber(s, numDelims);
			points.push_back(p);
		}

		return points;
	}
}

//================================================
// SVGTransform
// Transformation matrix
//================================================

namespace svg2b2d
{

    //
    // parsing transforms
    //
    static ByteSpan parseTransformArgs(const ByteSpan& inChunk, double* args, int maxNa, int& na)
    {
        na = 0;

        ByteSpan s = inChunk;

        // Skip past everything until we see a '('
		s = chunk_find_char(s, '(');

        // If we got to the end of the chunk, and did not see the '('
        // then just return
        if (!s)
            return s;

        // by the time we're here, we're sitting right on top of the 
        //'(', so skip past that to get to what should be the numbers
        s++;

        // We want to create a  chunk that contains all the numbers
        // without the closing ')', so create a chunk that just
        // represents that range.
        // Start the chunk at the current position
        // and expand it after we find the ')'
        ByteSpan item = s;
        item.fEnd = item.fStart;

        // scan until the closing ')'
        s = chunk_find_char(s, ')');


        // At this point, we're either sitting at the ')' or at the end
        // of the chunk.  If we're at the end of the chunk, then we
        // didn't find the closing ')', so just return
        if (!s)
            return s;

        // We found the closing ')', so if we use the current position
        // as the end (sitting on top of the ')', the item chunk will
        // perfectly represent the numbers we want to parse.
        item.fEnd = s.fStart;

        // Create a chunk that will represent a specific number to be parsed.
        ByteSpan numChunk{};

        // Move the source chunk cursor past the ')', so that whatever
        // needs to use it next is ready to go.
        s++;

        // Now we're ready to parse the individual numbers
		auto numDelims = wspChars + ',';
        
        while (item) {
            if (na >= maxNa)
                break;
            
			args[na++] = nextNumber(item, numDelims);
        }

        return s;
    }

    static ByteSpan parseMatrix(ByteSpan& inMatrix, BLMatrix2D &m)
    {
        ByteSpan s = inMatrix;
        m.reset();  // start with identity

        
        double t[6];    // storage for our 6 numbers
		int na = 0;     // Number of arguments parsed

        s = parseTransformArgs(s, t, 6, na);

        if (na != 6)
            return s;

		m.reset(t[0], t[1], t[2], t[3], t[4], t[5]);

        return s;
    }
    
    
    static ByteSpan parseTranslate(const ByteSpan& inChunk, BLMatrix2D& xform)
    {
        double args[2];
        int na = 0;
        ByteSpan s = inChunk;
        s = parseTransformArgs(s, args, 2, na);
        if (na == 1)
            args[1] = 0.0;

        xform.translate(args[0], args[1]);

        return s;
    }
    
    static ByteSpan parseScale(const ByteSpan& inChunk, BLMatrix2D& xform)
    {
        double args[2]{ 0 };
        int na = 0;
        ByteSpan s = inChunk;
        
        s = parseTransformArgs(s, args, 2, na);

        if (na == 1)
            args[1] = args[0];

        xform.scale(args[0], args[1]);

        return s;
    }
    
    
    static ByteSpan parseSkewX(const ByteSpan& inChunk, BLMatrix2D& xform)
    {
        double args[1];
        int na = 0;
        ByteSpan s = inChunk;
        s = parseTransformArgs(s, args, 1, na);

        xform.resetToSkewing(radians(args[0]), 0.0f);

        return s;
    }

    static ByteSpan parseSkewY(const ByteSpan& inChunk, BLMatrix2D& xform)
    {
        double args[1]{ 0 };
        int na = 0;
        ByteSpan s = inChunk;
        s = parseTransformArgs(s, args, 1, na);

        xform.resetToSkewing(0.0f, radians(args[0]));

        return s;
    }
    
    
    static ByteSpan parseRotate(const ByteSpan& inChunk, BLMatrix2D& xform)
    {
        double args[3]{ 0 };
        int na = 0;
        float m[6]{ 0 };
        float t[6]{ 0 };
        ByteSpan s = inChunk;

		s = parseTransformArgs(s, args, 3, na);

        if (na == 1)
            args[1] = args[2] = 0.0f;

		xform.rotate(radians(args[0]), args[1], args[2]);

        return  s;
    }


    struct SVGTransform : public SVGVisualProperty
    {
        BLMatrix2D fTransform{};

		SVGTransform(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
        SVGTransform(const SVGTransform& other)
            :SVGVisualProperty(other)
            ,fTransform(other.fTransform)
        {

        }

        SVGTransform& operator=(const SVGTransform& rhs)
        {
            SVGVisualProperty::operator=(rhs);
            fTransform = rhs.fTransform;

            return *this;
        }

        BLMatrix2D& getTransform() { return fTransform; }

		void loadSelfFromChunk(const ByteSpan& inChunk) override
		{
            ByteSpan s = inChunk;
            fTransform.reset();     // set to identity initially
            

            while (s)
            {
				s = chunk_skip_wsp(s);
                
                // Set out temp transform to the identity to start
                // so that if parsing goes wrong, we can still do
                // the multiply without worrying about messing things up
                // That means, the individula parsing functions need to not
                // partially mess up the transform if they fail.
                //svg_xformIdentity(t);

                BLMatrix2D tm{};
				tm.reset();

                if (chunk_starts_with_cstr(s, "matrix"))
                {
                    s = parseMatrix(s, tm);
                    fTransform = tm;
                    set(true);
                }
                else if (chunk_starts_with_cstr(s, "translate"))
                {
                    s = parseTranslate(s, tm);
                    fTransform.transform(tm);
                    set(true);
                }
                else if (chunk_starts_with_cstr(s, "scale"))
                {
                    s = parseScale(s, tm);
					fTransform.transform(tm);
                    set(true);
                }
                else if (chunk_starts_with_cstr(s, "rotate"))
                {
                    s = parseRotate(s, tm);
                    fTransform.transform(tm);
                    set(true);
                }
                else if (chunk_starts_with_cstr(s, "skewX"))
                {
                    s = parseSkewX(s,tm);
                    fTransform.transform(tm);
                    set(true);
                }
                else if (chunk_starts_with_cstr(s, "skewY"))
                {
                    s = parseSkewY(s,tm);
                    fTransform.transform(tm);
                    set(true);
                }
                else {
                    s++;
                }

            }

		}

        void drawSelf(BLContext& ctx) override
        {
			ctx.transform(fTransform);
        }

        static std::shared_ptr<SVGTransform> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
			std::shared_ptr<SVGTransform> tform = std::make_shared<SVGTransform>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                tform->loadFromChunk(inChunk);

            return tform;
        }

        // "transform"
        static std::shared_ptr<SVGTransform> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            return createFromChunk(root, name, elem.getAttribute(name));
        }
    };
    

}