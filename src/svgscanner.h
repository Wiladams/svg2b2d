#pragma once

// References
// https://github.com/lemire/fastbase64

#include "blend2d.h"

#include "maths.h"
#include "shaper.h"
#include "geometry.h"

#include "bspanutil.h"
#include "xmlscan.h"
#include "svgshapes.h"


#include <functional>
#include <charconv>
#include <string>
#include <array>
#include <vector>
#include <map>
#include <stack>
#include <memory>


namespace svg
{
    using namespace svg2b2d;
	using namespace ndt_debug;
    using std::string;
    using svg2b2d::ByteSpan;


    static charset digitChars("0123456789");
    static charset numberChars("0123456789.-+eE");


    constexpr auto SVG_MAX_ATTR = 128;
    constexpr auto SVG_MAX_DASHES = 8;

    // Turn a chunk into a vector of chunks, splitting on the delimiters
    // BUGBUG - should consider the option of empty chunks, especially at the boundaries
    static INLINE std::vector<DataChunk> chunk_split(const DataChunk& inChunk, const charset& delims, bool wantEmpties = false) noexcept
    {
        std::vector<DataChunk> result;
        DataChunk s = inChunk;
        while (s)
        {
            DataChunk token = chunk_token(s, delims);
            //if (size(token) > 0)
            result.push_back(token);
        }

        return result;
    }

}




