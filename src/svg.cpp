
#include "svg.h"
#include "svgscanner.h"

namespace svg2b2d {
    struct SVGDocument : public IDrawable
    {
		// All the drawable nodes within this document
        std::vector<std::shared_ptr<IDrawable>> fShapes{};
        BLRoundRect fExtent{};

        SVGDocument() = default;

        // When we want to render into a BLContext
        void draw(BLContext& ctx) override
        {
            // iterate over all the nodes, telling them to draw
            for (auto& shape : fShapes)
            {
                shape->draw(ctx);
            }
        }

        // Add a node that can be drawn
		void addNode(std::shared_ptr<SVGObject> node)
		{
			fShapes.push_back(node);
		}
        
        // Load the document from an XML Iterator
        // Since this is the top level document, we just want to kick
        // off loading the root node 'svg', and we're done 
        void loadFromIterator(XmlElementIterator& iter)
        {

            // skip past any elements that come before the 'svg' element
            while (iter)
            {
                const XmlElement& elem = *iter;

                if (!elem)
                    break;

                printXmlElement(*iter);

                // Skip over these node types we don't know how to process
                if (elem.isComment() || elem.isContent() || elem.isProcessingInstruction())
                {
                    iter++;
                    continue;
                }

                if (elem.isStart() && (elem.name() == "svg"))
                {
                    auto node = SVGRootNode::createFromIterator(iter);
                    if (node != nullptr)
                    {
                        addNode(node);
                    }
                }

                iter++;
            }
        }
        
        bool readFromData(const ByteSpan &inChunk)
        {
			DataChunk s = inChunk;
			//s = chunk_trim(s, wspChars);

			XmlElementIterator iter(s);

			loadFromIterator(iter);

			return true;
        }

    };
}


bool parseSVG(const ByteSpan &inChunk, BLImage &outImage)
{
    // Create a new document
    svg2b2d::SVGDocument doc;

    // Load the document from the data
    doc.readFromData(inChunk);

    // Draw the document into a BLContext
    BLContext ctx(outImage);
    doc.draw(ctx);
    ctx.end();
    
    return true;
}