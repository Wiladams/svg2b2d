#pragma once

#include "pathsegment.h"
#include "pathbuilder.h"

namespace svg2b2d
{

    // Take a vector of PathSegments and turn it into a BLPath object
    static bool blPathFromSegments(std::vector<svg2b2d::PathSegment>& segments, BLPath& apath)
    {
        PathBuilder builder{};

        builder.parseCommands(segments);
        apath = builder.getPath();

        return true;
    }

    //
    // Take a ByteSpan, that represents a path 'd' attribute, and turn it into a BLPath object
    //
    static bool blPathFromCommands(ByteSpan& chunk, BLPath& apath)
    {
        std::vector<svg2b2d::PathSegment> segments{};

        svg2b2d::tokenizePath(chunk, segments);

        return blPathFromSegments(segments, apath);
    }
}