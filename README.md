# svg2b2d
SVG parser, using blend2d as backend renderer


<img src="images/Accordion.png" alt="Accordion" width=340/></br>

look at projects/genimage to see how easy it is to construct a BLImage from an SVG

The main interface is in svg.h

```C
#pragma once

#include "blend2d.h"


#ifdef __cplusplus
extern "C" {
#endif

bool parseSVG(const void *bytes, const size_t sz, BLImage &outImage);

#ifdef __cplusplus
}
#endif
```

The process of using it is as simple as this:

```C
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
    BLImage outImage(1000, 1000, BL_FORMAT_PRGB32);

    // parse the svg, and draw it into the image
    parseSVG(mapped->data(), mapped->size(), outImage);
    
    // save the image to a png file
    const char *output = argc > 2 ? argv[2] : "output.png";
    outImage.writeToFile(output);

    // close the mapped file
    mapped->close();


    return 0;
}
```

The generated image will be the size of the .svg.

If you want to try the svg2png.exe it's like this:

svg2png.exe filename.svg  fileout.png

If you leave off the "fileout.png", it will use 'output.png' as the filename.

And that's it!
