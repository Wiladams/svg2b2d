
#include "svg.h"

#include "svgtypes.h"
#include "svgshapes.h"
#include "bspanutil.h"
#include "drawable.h"

#include <vector>
#include <memory>

using namespace svg2b2d;



bool parseSVG(const void* bytes, const size_t sz, BLImage& outImage)
{
    svg2b2d::ByteSpan inChunk = chunk_from_data_size(bytes, sz);
    
    // Create a new document
    SVGDocument doc;

    // Load the document from the data
    doc.readFromData(inChunk);

    // Draw the document into a BLContext
    BLContext ctx(outImage);
    doc.draw(ctx);
    ctx.end();
    
    return true;
}