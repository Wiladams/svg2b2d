#include "mmap.h"
#include "xmlscan.h"
#include "xmlutil.h"

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
    const char* filename = argv[1];
    auto mapped = mmap::createShared(filename);

    if (mapped == nullptr)
        return 0;


    // 
	// Parse the mapped file as XML
    // printing out the con
    ByteSpan s(mapped->data(), mapped->size());
    
    XmlElementIterator iter(s);

	// iterate over the elements
	// printing each one
    while (*iter)
    {
        XmlElement elem = *iter;

		ndt_debug::printXmlElement(elem);

        iter.next();
    }

    // close the mapped file
    mapped->close();


    return 0;
}
