#pragma once


#include "svgutils.h"
#include "blend2d.h"

#include <vector>

//
// PathContour - smallest addressable geometry
// PathSegment - a collection of contours
// Path        - a complete set of geometry
// Shape        - Geometry + drawing attributes
//

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

namespace svg2b2d
{
    //
    // PathBuilder
    // This object takes a series of path segments and turns them into a BLPath object
    //
    struct PathBuilder
    {
    public:

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
        virtual void finishSegment()
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

            finishSegment();

            fWorkingPath.moveTo(cmd.fNumbers[0], cmd.fNumbers[1]);


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

            finishSegment();

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

            double rx = cmd.fNumbers[0];
            double ry = cmd.fNumbers[1];
            double xRotation = cmd.fNumbers[2];
            double largeArcFlag = cmd.fNumbers[3];
            double sweepFlag = cmd.fNumbers[4];
            double x = lastPosition().x + cmd.fNumbers[5];
            double y = lastPosition().y + cmd.fNumbers[6];

            bool larc = largeArcFlag > 0.5f;
            bool swp = sweepFlag > 0.5f;
            double rotation = radians(xRotation);

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
            finishSegment();
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
            }

            finishSegment();

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
