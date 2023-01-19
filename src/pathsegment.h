#pragma once

#include "bspan.h"
#include <vector>

namespace svg2b2d
{
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


    //
    // tokenizePath
    // Given a ByteSpan, representing a series of path segments, create a vector of PathSegments.
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
                //printf("CMD: %c\n", *s);
                svg2b2d::PathSegment cmd{};
                cmd.fCommand = svg2b2d::SegmentKind(*s);
                commands.push_back(cmd);
                s++;

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
                    double anumber = 0;
                    anumber = toNumber(numChunk);
                    //std::from_chars((const char*)numChunk.fStart, (const char*)numChunk.fEnd, afloat);
                    commands.back().addNumber(anumber);
                    //printf("  %3.5f\n", afloat);
                }

            }
        }
    }
}
