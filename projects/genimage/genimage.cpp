

#include "blend2d.h"
#include "mmap.h"
#include "svg.h"

using namespace filemapper;
using namespace svg2b2d;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: genimage <svg file>  [output file]\n");
        return 1;
    }

    // create an mmap for the specified file
    const char *filename = argv[1];
    auto mapped = mmap::createShared(filename);

    if (mapped == nullptr)
        return 0;

    // Create the BLImage we're going to draw into
    BLImage outImage(420, 340, BL_FORMAT_PRGB32);

    // parse the svg, and draw it into the image
    parseSVG(mapped->data(), mapped->size(), outImage);
    
    // save the image to a png file
    const char *output = argc > 2 ? argv[2] : "output.png";
    outImage.writeToFile(output);

    // close the mapped file
    mapped->close();


    return 0;
}