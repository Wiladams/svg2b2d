#pragma once

#include "blend2d.h"
#include "charset.h"
#include "bspanutil.h"


#include <map>
#include <functional>


	//
	// Parse a string representing a svg <path> 'd' attribute and
	// turn it directly into a BLPath object
	// return true on success, false otherwise 
	// Aside from parsing SVG element structure, this is one 
	// of the most complex functions in the library
namespace svg2b2d {
		static charset whitespaceChars(",\t\n\f\r ");          // whitespace found in paths
		static charset commandChars("mMlLhHvVcCqQsStTaAzZ");   // set of characters used for commands
		static charset numberChars("0123456789.+-eE");         // digits, symbols, and letters found in numbers
		static charset leadingChars("0123456789.+-");          // digits, symbols, and letters found in numbers
		static charset digitChars("0123456789");                   // only digits


		    // Shaper contour Commands
    // Origin from SVG path commands
    // M - move       (M, m)
    // L - line       (L, l, H, h, V, v)
    // C - cubic      (C, c, S, s)
    // Q - quad       (Q, q, T, t)
    // A - ellipticArc  (A, a)
    // Z - close        (Z, z)
    enum class SegmentCommand : uint8_t
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

		// Consume the next number off the front of the chunk
		// modifying the input chunk to advance past the  number
		// we removed.
		// Return true if we found a number, false otherwise
		// BUGBUG - bspanutil now has parseNextNumber, so this can be replaced
		/*
		static bool parseNumber(ByteSpan& s, double& outNumber)
		{
			// clear up leading whitespace, including ','
			s = chunk_ltrim(s, whitespaceChars);

			ByteSpan numChunk{};
			s = scanNumber(s, numChunk);

			if (!numChunk)
				return false;

			outNumber = toNumber(numChunk);

			return true;
		}
		*/
		static bool parseMoveTo(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x{ 0 };
			double y{ 0 };

			if (!parseNextNumber(s, x))
				return false;
			if (!parseNextNumber(s, y))
				return false;

			if (iteration == 0)
				apath.moveTo(x, y);
			else
				apath.lineTo(x, y);

			iteration++;

			return true;
		}

		static bool parseMoveBy(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x{ 0 };
			double y{ 0 };

			if (!parseNextNumber(s, x))
				return false;
			if (!parseNextNumber(s, y))
				return false;

			BLPoint lastPos{};
			apath.getLastVertex(&lastPos);

			if (iteration == 0)
				apath.moveTo(lastPos.x + x, lastPos.y + y);
			else
				apath.lineTo(lastPos.x + x, lastPos.y + y);

			iteration++;

			return true;
		}

		static bool parseLineTo(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x{ 0 };
			double y{ 0 };

			if (!parseNextNumber(s, x))
				return false;
			if (!parseNextNumber(s, y))
				return false;

			apath.lineTo(x, y);

			iteration++;

			return true;
		}

		static bool parseLineBy(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x{ 0 };
			double y{ 0 };

			if (!parseNextNumber(s, x))
				return false;
			if (!parseNextNumber(s, y))
				return false;

			BLPoint lastPos{};
			apath.getLastVertex(&lastPos);

			apath.lineTo(lastPos.x + x, lastPos.y + y);

			iteration++;

			return true;
		}

		static bool parseHLineTo(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x{ 0 };

			if (!parseNextNumber(s, x))
				return false;

			BLPoint lastPos{};
			apath.getLastVertex(&lastPos);
			apath.lineTo(x, lastPos.y);

			iteration++;

			return true;
		}

		static bool parseHLineBy(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x{ 0 };

			if (!parseNextNumber(s, x))
				return false;

			BLPoint lastPos{};
			apath.getLastVertex(&lastPos);
			apath.lineTo(lastPos.x + x, lastPos.y);

			iteration++;

			return true;
		}

		static bool parseVLineTo(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double y{ 0 };

			if (!parseNextNumber(s, y))
				return false;

			BLPoint lastPos{};
			apath.getLastVertex(&lastPos);
			apath.lineTo(lastPos.x, y);

			iteration++;

			return true;
		}

		static bool parseVLineBy(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double y{ 0 };

			if (!parseNextNumber(s, y))
				return false;

			BLPoint lastPos{};
			apath.getLastVertex(&lastPos);
			apath.lineTo(lastPos.x, lastPos.y + y);

			iteration++;

			return true;
		}

		// Command - Q
		static bool parseQuadTo(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x1{ 0 };
			double y1{ 0 };
			double x2{ 0 };
			double y2{ 0 };

			if (!parseNextNumber(s, x1))
				return false;
			if (!parseNextNumber(s, y1))
				return false;
			if (!parseNextNumber(s, x2))
				return false;
			if (!parseNextNumber(s, y2))
				return false;

			apath.quadTo(x1, y1, x2, y2);

			iteration++;

			return true;
		}

		// Command - q
		static bool parseQuadBy(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x1{ 0 };
			double y1{ 0 };
			double x2{ 0 };
			double y2{ 0 };

			if (!parseNextNumber(s, x1))
				return false;
			if (!parseNextNumber(s, y1))
				return false;
			if (!parseNextNumber(s, x2))
				return false;
			if (!parseNextNumber(s, y2))
				return false;

			BLPoint lastPos{};
			apath.getLastVertex(&lastPos);

			apath.quadTo(lastPos.x + x1, lastPos.y + y1, lastPos.x + x2, lastPos.y + y2);

			iteration++;

			return true;
		}

		// Command - T
		static bool parseSmoothQuadTo(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x2{ 0 };
			double y2{ 0 };

			if (!parseNextNumber(s, x2))
				return false;
			if (!parseNextNumber(s, y2))
				return false;

			apath.smoothQuadTo(x2, y2);

			iteration++;

			return true;
		}

		// Command - t
		static bool parseSmoothQuadBy(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x2{ 0 };
			double y2{ 0 };

			if (!parseNextNumber(s, x2))
				return false;
			if (!parseNextNumber(s, y2))
				return false;

			BLPoint lastPos{};
			apath.getLastVertex(&lastPos);

			apath.smoothQuadTo(lastPos.x + x2, lastPos.y + y2);

			iteration++;

			return true;
		}

		// Command - C
		static bool parseCubicTo(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x1{ 0 };
			double y1{ 0 };
			double x2{ 0 };
			double y2{ 0 };
			double x3{ 0 };
			double y3{ 0 };

			if (!parseNextNumber(s, x1))
				return false;
			if (!parseNextNumber(s, y1))
				return false;
			if (!parseNextNumber(s, x2))
				return false;
			if (!parseNextNumber(s, y2))
				return false;
			if (!parseNextNumber(s, x3))
				return false;
			if (!parseNextNumber(s, y3))
				return false;

			apath.cubicTo(x1, y1, x2, y2, x3, y3);

			iteration++;

			return true;
		}

		// Command - c
		static bool parseCubicBy(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x1{ 0 };
			double y1{ 0 };
			double x2{ 0 };
			double y2{ 0 };
			double x3{ 0 };
			double y3{ 0 };

			if (!parseNextNumber(s, x1))
				return false;
			if (!parseNextNumber(s, y1))
				return false;
			if (!parseNextNumber(s, x2))
				return false;
			if (!parseNextNumber(s, y2))
				return false;
			if (!parseNextNumber(s, x3))
				return false;
			if (!parseNextNumber(s, y3))
				return false;

			BLPoint lastPos{};
			apath.getLastVertex(&lastPos);

			apath.cubicTo(lastPos.x + x1, lastPos.y + y1, lastPos.x + x2, lastPos.y + y2, lastPos.x + x3, lastPos.y + y3);

			iteration++;

			return true;
		}

		// Command - S
		static bool parseSmoothCubicTo(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x2{ 0 };
			double y2{ 0 };
			double x3{ 0 };
			double y3{ 0 };

			if (!parseNextNumber(s, x2))
				return false;
			if (!parseNextNumber(s, y2))
				return false;
			if (!parseNextNumber(s, x3))
				return false;
			if (!parseNextNumber(s, y3))
				return false;

			apath.smoothCubicTo(x2, y2, x3, y3);

			iteration++;

			return true;
		}

		// Command - s
		static bool parseSmoothCubicBy(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double x2{ 0 };
			double y2{ 0 };
			double x3{ 0 };
			double y3{ 0 };

			if (!parseNextNumber(s, x2))
				return false;
			if (!parseNextNumber(s, y2))
				return false;
			if (!parseNextNumber(s, x3))
				return false;
			if (!parseNextNumber(s, y3))
				return false;

			BLPoint lastPos{};
			apath.getLastVertex(&lastPos);

			apath.smoothCubicTo(lastPos.x + x2, lastPos.y + y2, lastPos.x + x3, lastPos.y + y3);

			iteration++;

			return true;
		}


		// Command - A
		static bool parseArcTo(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double rx{ 0 };
			double ry{ 0 };
			double xAxisRotation{ 0 };
			double largeArcFlag{ 0 };
			double sweepFlag{ 0 };
			double x{ 0 };
			double y{ 0 };

			if (!parseNextNumber(s, rx))
				return false;
			if (!parseNextNumber(s, ry))
				return false;
			if (!parseNextNumber(s, xAxisRotation))
				return false;
			if (!parseNextNumber(s, largeArcFlag))
				return false;
			if (!parseNextNumber(s, sweepFlag))
				return false;
			if (!parseNextNumber(s, x))
				return false;
			if (!parseNextNumber(s, y))
				return false;

			bool larc = largeArcFlag > 0.5f;
			bool swp = sweepFlag > 0.5f;
			double xrot = radians(xAxisRotation);

			apath.ellipticArcTo(BLPoint(rx, ry), xrot, larc, swp, BLPoint(x, y));

			iteration++;

			return true;
		}

		// Command - a
		static bool parseArcBy(ByteSpan& s, BLPath& apath, int& iteration)
		{
			double rx{ 0 };
			double ry{ 0 };
			double xAxisRotation{ 0 };
			double largeArcFlag{ 0 };
			double sweepFlag{ 0 };
			double x{ 0 };
			double y{ 0 };

			if (!parseNextNumber(s, rx))
				return false;
			if (!parseNextNumber(s, ry))
				return false;
			if (!parseNextNumber(s, xAxisRotation))
				return false;
			if (!parseNextNumber(s, largeArcFlag))
				return false;
			if (!parseNextNumber(s, sweepFlag))
				return false;
			if (!parseNextNumber(s, x))
				return false;
			if (!parseNextNumber(s, y))
				return false;

			bool larc = largeArcFlag > 0.5f;
			bool swp = sweepFlag > 0.5f;
			double xrot = radians(xAxisRotation);

			BLPoint lastPos{};
			apath.getLastVertex(&lastPos);

			apath.ellipticArcTo(BLPoint(rx, ry), xrot, larc, swp, BLPoint(lastPos.x + x, lastPos.y + y));

			iteration++;

			return true;
		}

		// Command - Z, z
		static bool parseClose(ByteSpan& s, BLPath& apath, int& iteration)
		{
			apath.close();
			// No parameters expected to follow, so don't
			// increment iteration
			return true;
		}


		// A dispatch std::map that matches the command character to the
		// appropriate parse function
		static std::map<SegmentCommand, std::function<bool(ByteSpan&, BLPath&, int&)>> parseMap = {
			{SegmentCommand::MoveTo, parseMoveTo},
			{SegmentCommand::MoveBy, parseMoveBy},
			{SegmentCommand::LineTo, parseLineTo},
			{SegmentCommand::LineBy, parseLineBy},
			{SegmentCommand::HLineTo, parseHLineTo},
			{SegmentCommand::HLineBy, parseHLineBy},
			{SegmentCommand::VLineTo, parseVLineTo},
			{SegmentCommand::VLineBy, parseVLineBy},
			{SegmentCommand::CubicTo, parseCubicTo},
			{SegmentCommand::CubicBy, parseCubicBy},
			{SegmentCommand::SCubicTo, parseSmoothCubicTo},
			{SegmentCommand::SCubicBy, parseSmoothCubicBy},
			{SegmentCommand::QuadTo, parseQuadTo},
			{SegmentCommand::QuadBy, parseQuadBy},
			{SegmentCommand::SQuadTo, parseSmoothQuadTo},
			{SegmentCommand::SQuadBy, parseSmoothQuadBy},
			{SegmentCommand::ArcTo, parseArcTo},
			{SegmentCommand::ArcBy, parseArcBy},
			{SegmentCommand::CloseTo, parseClose},
			{SegmentCommand::CloseBy, parseClose}
		};



		static bool parsePath(const ByteSpan& inSpan, BLPath& apath)
		{
			// Use a ByteSpan as a cursor on the input
			ByteSpan s = inSpan;
			SegmentCommand currentCommand = SegmentCommand::INVALID;
			int iteration = 0;

			while (s)
			{
				// ignore leading whitespace
				s = chunk_ltrim(s, whitespaceChars);

				// If we've gotten to the end, we're done
				// so just return
				if (!s)
					break;

				if (commandChars[*s])
				{
					// we have a command
					currentCommand = SegmentCommand(*s);
					iteration = 0;
					s++;
				}

				// Use parseMap to dispatch to the appropriate
				// parse function
				if (!parseMap[currentCommand](s, apath, iteration))
					return false;


			}

			return true;
		}
	}
