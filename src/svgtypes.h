#pragma once


#include "svgutils.h"
#include "svgcolors.h"

#include <memory>
#include <vector>
#include <cstdint>		// uint8_t, etc
#include <cstddef>		// nullptr_t, ptrdiff_t, size_t


namespace svg2b2d {
	struct IMapSVGNodes;    // forward declaration
    
    // IDrawable
    // Base interface for anything that might have an effect
    // on a drawing context.
    // 
    struct IDrawable
    {
        virtual ~IDrawable() {}

        virtual void draw(BLContext& ctx) = 0;
    };
    
    struct SVGObject : public IDrawable
    {
        XmlElement fSourceElement;
        
        IMapSVGNodes* fRoot{ nullptr };
        std::string fName{};    // The tag name of the element
        BLVar fVar{};
        bool fIsVisible{ false };
        BLBox fExtent{};

        
        
		SVGObject() = delete;
        SVGObject(const SVGObject& other) :fName(other.fName) {}
        SVGObject(IMapSVGNodes* root) :fRoot(root) {}
		virtual ~SVGObject() = default;
        
		SVGObject& operator=(const SVGObject& other) {
            fRoot = other.fRoot;
            fName = other.fName;
			BLVar fVar = other.fVar;
            
            return *this;
		}
        
		IMapSVGNodes* root() const { return fRoot; }
        virtual void setRoot(IMapSVGNodes* root) { fRoot = root; }
        
        const std::string& name() const { return fName; }
        void setName(const std::string& name) { fName = name; }

		const bool visible() const { return fIsVisible; }
		void setVisible(bool visible) { fIsVisible = visible; }
        
        const XmlElement& sourceElement() const { return fSourceElement; }
        
        // sub-classes should return something interesting as BLVar
        // This can be used for styling, so images, colors, patterns, gradients, etc
        virtual const BLVar& getVariant()
        {
            return fVar;
        }
        
        void draw(BLContext& ctx) override
        {
            ;// draw the object
        }
        
        virtual void loadSelfFromXml(const XmlElement& elem)
        {
            ;
        }

        virtual void loadFromXmlElement(const svg2b2d::XmlElement& elem)
        {
            fSourceElement = elem;
            
            // load the common attributes
            setName(elem.name());

            // call to loadselffromxml
            // so sub-class can do its own loading
            loadSelfFromXml(elem);
        }
    };
    
    // Core interface to hold document level state, primarily
    // for the purpose of looking up nodes.
    struct IMapSVGNodes
    {
        virtual std::shared_ptr<SVGObject> findNodeById(const std::string& name) = 0;
        virtual std::shared_ptr<SVGObject> findNodeByHref(const ByteSpan& href) = 0;

        
        virtual void addDefinition(const std::string& name, std::shared_ptr<SVGObject> obj) = 0;

        virtual void setInDefinitions(bool indefs) = 0;
        virtual bool inDefinitions() const = 0;
    };
    
    
    // SVGVisualProperty
    // This is meant to be the base class for things that are optionally
    // used to alter the graphics context.
    // If isSet() is true, then the drawSelf() is called.
	// sub-classes should override drawSelf() to do the actual drawing
    //
    // This is used for things like; Paint, Transform, Miter, etc.
    //
    struct SVGVisualProperty :  public SVGObject
    {
        bool fIsSet{ false };

        //SVGVisualProperty() :SVGObject(),fIsSet(false){}
        SVGVisualProperty(IMapSVGNodes *root):SVGObject(root),fIsSet(false){}
        SVGVisualProperty(const SVGVisualProperty& other)
            :SVGObject(other)
            ,fIsSet(other.fIsSet)
        {}

        SVGVisualProperty operator=(const SVGVisualProperty& rhs)
        {
            SVGObject::operator=(rhs);
            fIsSet = rhs.fIsSet;
            
            return *this;
        }

        void set(const bool value) { fIsSet = value; }
        bool isSet() const { return fIsSet; }

		virtual void loadSelfFromChunk(const ByteSpan& chunk)
        {
			;
        }

        virtual void loadFromChunk(const ByteSpan& chunk)
        {
			loadSelfFromChunk(chunk);
        }
        
        // Apply propert to the context conditionally
        virtual void drawSelf(BLContext& ctx)
        {
            ;
        }

        void draw(BLContext& ctx) override
        {
            if (isSet())
                drawSelf(ctx);
        }

    };
}



// Specific types of attributes
//==============================================================================
// SVGDimension
// used for length, time, frequency, resolution, location
//==============================================================================
namespace svg2b2d
{

    struct SVGPoint {
        float fX;
        float fY;

        SVGPoint(float x, float y) : fX(x), fY(y) {}
        SVGPoint() : fX(0), fY(0) {}

        float x() const { return fX; }
        float y() const { return fY; }

        static SVGPoint fromChunk(const ByteSpan& inChunk)
        {
            SVGPoint point{};

            ByteSpan s = inChunk;
            ByteSpan numChunk;
            charset numDelims = wspChars + ',';

            point.fX = (float)nextNumber(s, numDelims);
            point.fY = (float)nextNumber(s, numDelims);

            return point;
        }

    };
}


namespace svg2b2d {
    struct SVGOpacity : public SVGVisualProperty
    {
        double fValue{ 1.0 };

		SVGOpacity(IMapSVGNodes* iMap):SVGVisualProperty(iMap){}


        void drawSelf(BLContext& ctx)
        {
			SVGVisualProperty::drawSelf(ctx);
			ctx.setFillAlpha(fValue);
        }

		void loadSelfFromChunk(const ByteSpan& inChunk)
		{
            fValue = parseDimension(inChunk).calculatePixels(1);
            set(true);
		}

        static std::shared_ptr<SVGOpacity> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
            std::shared_ptr<SVGOpacity> sw = std::make_shared<SVGOpacity>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                sw->loadFromChunk(inChunk);

            return sw;
        }

        static std::shared_ptr<SVGOpacity> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            return createFromChunk(root, name, elem.getAttribute(name));
        }
	};

}


namespace svg2b2d {
    struct SVGFontSize : public SVGVisualProperty
    {
        double fValue{ 12.0 };

        //SVGFontSize() : SVGVisualProperty() {}
		SVGFontSize(IMapSVGNodes* inMap) : SVGVisualProperty(inMap) {}
        SVGFontSize(const SVGFontSize& other) :SVGVisualProperty(other) { fValue = other.fValue; }
        
        SVGFontSize& operator=(const SVGFontSize& rhs)
        {
            SVGVisualProperty::operator=(rhs);
            fValue = rhs.fValue;
            return *this;
        }

        void drawSelf(BLContext& ctx)
        {
            //ctx.textSize(fValue);
        }

        void loadSelfFromChunk(const ByteSpan& inChunk)
        {
            fValue = parseDimension(inChunk).calculatePixels(96);
            set(true);
        }

        static std::shared_ptr<SVGFontSize> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
            std::shared_ptr<SVGFontSize> sw = std::make_shared<SVGFontSize>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                sw->loadFromChunk(inChunk);

            return sw;
        }

        static std::shared_ptr<SVGFontSize> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            return createFromChunk(root, name, elem.getAttribute(name));
        }
    };


    
    // Text Alignment
enum class ALIGNMENT : unsigned
{
    MIDDLE = 0x01,
    START = 0x02,
    END = 0x04,
} ;
    
    struct SVGTextAnchor : public SVGVisualProperty
    {
        ALIGNMENT fValue{ ALIGNMENT::START };
        
        //SVGTextAnchor() : SVGVisualProperty() {}
		SVGTextAnchor(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
        SVGTextAnchor(const SVGTextAnchor& other) :SVGVisualProperty(other) { fValue = other.fValue; }
        
        SVGTextAnchor& operator=(const SVGTextAnchor& rhs)
        {
            SVGVisualProperty::operator=(rhs);
            fValue = rhs.fValue;
            return *this;
        }

        void drawSelf(BLContext& ctx)
        {
            // BUGBUG, need to calculate alignment
            //ctx.textAlign(fValue, ALIGNMENT::BASELINE);
        }

        void loadSelfFromChunk(const ByteSpan& inChunk)
        {
            if (inChunk == "start")
                fValue = ALIGNMENT::START;
			else if (inChunk == "middle")
				fValue = ALIGNMENT::MIDDLE;
			else if (inChunk == "end")
				fValue = ALIGNMENT::END;


            set(true);
        }

        static std::shared_ptr<SVGTextAnchor> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
            std::shared_ptr<SVGTextAnchor> sw = std::make_shared<SVGTextAnchor>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                sw->loadFromChunk(inChunk);

            return sw;
        }

        static std::shared_ptr<SVGTextAnchor> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            return createFromChunk(root, name, elem.getAttribute(name));
        }
    };
    
    struct SVGTextAlign : public SVGVisualProperty
    {
        ALIGNMENT fValue{ ALIGNMENT::START };

		SVGTextAlign(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
        SVGTextAlign(const SVGTextAlign& other) :SVGVisualProperty(other) { fValue = other.fValue; }
        
        SVGTextAlign& operator=(const SVGTextAlign& rhs)
        {
            SVGVisualProperty::operator=(rhs);
            fValue = rhs.fValue;
            return *this;
        }

        void drawSelf(BLContext& ctx)
        {
			// BUGBUG, need to calculate alignment
            //ctx.textAlign(fValue, ALIGNMENT::BASELINE);
        }

        void loadSelfFromChunk(const ByteSpan& inChunk)
        {
            if (inChunk == "center")
				fValue = ALIGNMENT::MIDDLE;
            
            set(true);
        }

        static std::shared_ptr<SVGTextAlign> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
            std::shared_ptr<SVGTextAlign> sw = std::make_shared<SVGTextAlign>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                sw->loadFromChunk(inChunk);

            return sw;
        }

        static std::shared_ptr<SVGTextAlign> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            return createFromChunk(root, name, elem.getAttribute(name));
        }
    };
}


namespace svg2b2d {
    void parseStyleAttribute(const ByteSpan & inChunk, XmlElement &styleElement)
    {
        // Turn the style element into attributes of an XmlElement, 
        // then, the caller can use that to more easily parse whatever they're
        // looking for.
        ByteSpan styleChunk = inChunk;
        
        if (styleChunk) {
            // use CSSInlineIterator to iterate through the key value pairs
            // creating a visual attribute, using the gSVGPropertyCreation map
            CSSInlineStyleIterator iter(styleChunk);

            while (iter.next())
            {
                std::string name = std::string((*iter).first.fStart, (*iter).first.fEnd);
                if (!name.empty() && (*iter).second)
                {
                    styleElement.addAttribute(name, (*iter).second);
                }
            }

        }

        return;
    }
}

//======================================================
// Definition of SVG Paint
//======================================================
namespace svg2b2d {
    // Representation of color according to CSS specification
	// https://www.w3.org/TR/css-color-4/#typedef-color
    // Over time, this structure could represent the full specification
    // but for practical purposes, we'll focus on rgb, rgba for now
    /*
    <color> = <absolute-color-base> | currentcolor | <system-color> 

    <absolute-color-base> = <hex-color> | <absolute-color-function> | <named-color> | transparent
    <absolute-color-function> = <rgb()> | <rgba()> |
                            <hsl()> | <hsla()> | <hwb()> |
                            <lab()> | <lch()> | <oklab()> | <oklch()> |
                            <color()>
    */

}

namespace svg2b2d {

    static BLRgba32 parseColorHex(const ByteSpan& chunk)
    {
        // BUGBUG - making a copy of the chunk to use sscanf
        char str[64];
        copy_to_cstr(str, 63, chunk);

        unsigned int r = 0, g = 0, b = 0;
        if (sscanf_s(str, "#%2x%2x%2x", &r, &g, &b) == 3)		// 2 digit hex
            return BLRgba32(r, g, b);
        if (sscanf_s(str, "#%1x%1x%1x", &r, &g, &b) == 3)		// 1 digit hex, e.g. #abc -> 0xccbbaa
            return BLRgba32(r * 17, g * 17, b * 17);			    // same effect as (r<<4|r), (g<<4|g), ..

        // if not one of those cases, just return a mid-gray
        return BLRgba32(128, 128, 128);
    }



    // Parse rgb color. The pointer 'str' must point at "rgb(" (4+ characters).
    // This function returns gray (rgb(128, 128, 128) == '#808080') on parse errors
    // for backwards compatibility. Note: other image viewers return black instead.

    static BLRgba32 parseColorRGB(const ByteSpan& inChunk)
    {
        // skip past the leading "rgb("
        ByteSpan s = inChunk;
        auto leading = chunk_token(s, "(");

        // s should now point to the first number
        // and 'leading' should contain 'rgb'
        // BUGBUG - we can check 'leading' to see if it's actually 'rgb'
        // but we'll just assume it is for now

        // get the numbers by separating at the ')'
        auto nums = chunk_token(s, ")");

        // So, now nums contains the individual numeric values, separated by ','
        // The individual numeric values are either
        // 50%
        // 50

        int i = 0;
        uint8_t rgbi[4]{};

        // Get the first token, which is red
        // if it's not there, then return gray
        auto num = chunk_token(nums, ",");
        if (chunk_size(num) < 1)
            return BLRgba32(128, 128, 128, 0);

        while (num)
        {
            auto cv = parseDimension(num);

            //writeChunk(num);
            if (chunk_find_char(num, '%'))
            {
                // it's a percentage
                // BUGBUG - we're assuming it's a range of [0..255]

                rgbi[i] = (uint8_t)(cv.value() / 100.0f * 255.0f);
            }
            else
            {
                // it's a regular number
                if (i==3)
                    rgbi[i] = (uint8_t)(cv.value()  * 255.0f);
                else 
                    rgbi[i] = (uint8_t)cv.value();
            }
            i++;
            num = chunk_token(nums, ",");
        }

        if (i == 4)
            return BLRgba32(rgbi[0], rgbi[1], rgbi[2], rgbi[3]);

        return BLRgba32(rgbi[0], rgbi[1], rgbi[2]);
    }

    static BLRgba32 parseColorName(const ByteSpan& inChunk)
    {
        std::string cName = std::string(inChunk.fStart, inChunk.fEnd);

        // If named color not found
        // or name == "none"
        // return fully transparent black
        // BUGBUG - this is different than not having a color attribute
        // in which case, we might want to eliminate color, and allow ancestor's color to come through
        if (!svg::colors.contains(cName))
            return BLRgba32(128, 128, 128, 255);

        return svg::colors[cName];
    }



    constexpr int SVG_PaintForFill = 1;
    constexpr int SVG_PaintForStroke = 2;

    struct SVGPaint : public SVGVisualProperty
    {
        BLVar fPaint{};
        bool fExplicitNone{ false };
        int fPaintFor{ SVG_PaintForFill };

		SVGPaint(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
        SVGPaint(const SVGPaint& other) :SVGVisualProperty(other) 
        {

        }

        SVGPaint& operator=(const SVGPaint& rhs)
        {
            SVGVisualProperty::operator=(rhs);

            blVarAssignWeak(&fPaint, &rhs.fPaint);
            fExplicitNone = rhs.fExplicitNone;
            fPaintFor = rhs.fPaintFor;

            return *this;
        }

        const BLVar& getVariant() override
        {
            return fPaint;
        }
        
        void setPaintFor(int pfor) { fPaintFor = pfor; }
        void setOpacity(double opacity)
        {
            uint32_t outValue;
            if (BL_SUCCESS == blVarToRgba32(&fPaint, &outValue))
            {
                BLRgba32 newColor(outValue);
                newColor.setA((uint32_t)(opacity * 255));
                blVarAssignRgba32(&fPaint, newColor.value);
            }
        }

        // Need to distinguish which function gets called
        // BUGBUG
        void drawSelf(BLContext& ctx)
        {
            switch (fPaintFor)
            {

            case SVG_PaintForFill:
                if (fExplicitNone)
                    ctx.setFillStyle(BLRgba32(0));
                else
                    ctx.setFillStyle(fPaint);
                break;

            case SVG_PaintForStroke:
                if (fExplicitNone)
					ctx.setStrokeStyle(BLRgba32(0));
                else
                    ctx.setStrokeStyle(fPaint);
                break;
            }

        }

        void loadFromUrl(const ByteSpan &inChunk)
        {
            ByteSpan str = inChunk;
            
            // the id we want should look like this
            // url(#id)
            // so we need to skip past the 'url(#'
            // and then find the closing ')'
            // and then we have the id
            auto url = chunk_token(str, "(");
            auto id = chunk_trim(chunk_token(str, ")"), wspChars);

            // The first character could be '.' or '#'
            // so we need to skip past that
            if (*id == '.' || *id == '#')
                id++;

            if (!id)
                return;

            // lookup the thing we're referencing
            std::string idStr = toString(id);

            if (fRoot != nullptr)
            {
                auto node = fRoot->findNodeById(idStr);
                
                // pull out the color value
                if (node != nullptr)
                {
                    const BLVar& aVar = node->getVariant();

                    auto res = blVarAssignWeak(&fPaint, &aVar);
                    if (res == BL_SUCCESS)
                        set(true);
                }
            }
        }
        
        void loadSelfFromChunk (const ByteSpan& inChunk)
        {
            BLRgba32 c{};
            
            blVarAssignRgba32(&fPaint, c.value);

            ByteSpan str = inChunk;
            ByteSpan rgbStr = chunk_from_cstr("rgb(");
            ByteSpan rgbaStr = chunk_from_cstr("rgba(");

            size_t len = 0;

            len = chunk_size(str);
            if (len >= 1 && *str == '#')
            {
                c = parseColorHex(str);
                blVarAssignRgba32(&fPaint, c.value);
                set(true);
            }
            else if (chunk_starts_with(str, rgbStr) || chunk_starts_with(str, rgbaStr))
            {
                c = parseColorRGB(str);
                blVarAssignRgba32(&fPaint, c.value);
                set(true);
            }
            else if (chunk_starts_with_cstr(str, "url(")) 
            {
                loadFromUrl(str);
            }
            else {
                std::string cName = std::string(str.fStart, str.fEnd);
                if (cName == "none") {
                    fExplicitNone = true;
                    set(true);
                }
                else if (svg::colors.contains(cName))
                {
                    c = svg::colors[cName];
                    blVarAssignRgba32(&fPaint, c.value);
                    set(true);
                }
                else {
                    // user wants some sort of color, which is either invalid name
                    // or a color function we don't support yet
                    // so set a default gray color
                    c = BLRgba32(128, 128, 128);
                    blVarAssignRgba32(&fPaint, c.value);
                    set(true);
                }
            }


        }

        static std::shared_ptr<SVGPaint> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
            std::shared_ptr<SVGPaint> paint = std::make_shared<SVGPaint>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                paint->loadFromChunk(inChunk);

            return paint;
        }

        static std::shared_ptr<SVGPaint> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            auto paint = createFromChunk(root, name, elem.getAttribute(name));

			if (!paint->isSet())
				return paint;
            
            if (name == "fill")
            {
                paint->setPaintFor(SVG_PaintForFill);
                // look for fill-opacity as well
                auto o = elem.getAttribute("fill-opacity");
                if (o)
                {
                    auto onum = toNumber(o);
                    paint->setOpacity(onum);
                }
            }
            else if (name == "stroke")
            {
                paint->setPaintFor(SVG_PaintForStroke);
                // look for stroke-opacity as well
                auto o = elem.getAttribute("stroke-opacity");
                if (o)
                {
                    auto onum = toNumber(o);
                    paint->setOpacity(onum);
                }
            }
			else if (name == "stop-color")
			{
				//paint->setPaintFor(SVG_PaintForStopColor);
				// look for stop-opacity as well
				auto o = elem.getAttribute("stop-opacity");
				if (o)
				{
					auto onum = toNumber(o);
					paint->setOpacity(onum);
				}
			}


            return paint;
        }
    };
}


namespace svg2b2d {

    //=========================================================
    // SVGFillRule
    //=========================================================
    struct SVGFillRule : public SVGVisualProperty
    {
		BLFillRule fValue{ BL_FILL_RULE_NON_ZERO };

   
        //SVGFillRule() : SVGVisualProperty() {}
		SVGFillRule(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
        SVGFillRule(const SVGFillRule& other) :SVGVisualProperty(other)
        {
            fValue = other.fValue;
        }

        SVGFillRule& operator=(const SVGFillRule& rhs)
        {
            SVGVisualProperty::operator=(rhs);
            fValue = rhs.fValue;
            return *this;
        }

        void drawSelf(BLContext& ctx)
        {
			ctx.setFillRule(fValue);
        }

        void loadSelfFromChunk(const ByteSpan& inChunk)
        {
            ByteSpan s = chunk_trim(inChunk, wspChars);

            set(true);

            if (s == "nonzero")
                fValue = BL_FILL_RULE_NON_ZERO;
            else if (s == "evenodd")
                fValue = BL_FILL_RULE_EVEN_ODD;
            else
                set(false);
        }

        static std::shared_ptr<SVGFillRule> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
            std::shared_ptr<SVGFillRule> node = std::make_shared<SVGFillRule>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                node->loadFromChunk(inChunk);

            return node;
        }

        static std::shared_ptr<SVGFillRule> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            return createFromChunk(root, name, elem.getAttribute(name));
        }
    };
}

namespace svg2b2d {
    //=========================================================
    // SVGStrokeWidth
    //=========================================================
    
    struct SVGStrokeWidth : public SVGVisualProperty
    {
		double fWidth{ 1.0};

		//SVGStrokeWidth() : SVGVisualProperty() {}
		SVGStrokeWidth(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
		SVGStrokeWidth(const SVGStrokeWidth& other) :SVGVisualProperty(other) { fWidth = other.fWidth; }
        
		SVGStrokeWidth& operator=(const SVGStrokeWidth& rhs)
		{
			SVGVisualProperty::operator=(rhs);
			fWidth = rhs.fWidth;
			return *this;
		}

		void drawSelf(BLContext& ctx)
		{
			ctx.setStrokeWidth(fWidth);
		}

		void loadSelfFromChunk(const ByteSpan& inChunk)
		{
			fWidth = toNumber(inChunk);
			set(true);
		}

		static std::shared_ptr<SVGStrokeWidth> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
		{
			std::shared_ptr<SVGStrokeWidth> sw = std::make_shared<SVGStrokeWidth>(root);

			// If the chunk is empty, return immediately 
			if (inChunk)
				sw->loadFromChunk(inChunk);

			return sw;
		}

		static std::shared_ptr<SVGStrokeWidth> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
		{
			return createFromChunk(root, name, elem.getAttribute(name));
		}
    };
    
    //=========================================================
    ///  SVGStrokeMiterLimit
	/// A visual property to set the miter limit for a stroke
    //=========================================================
    struct SVGStrokeMiterLimit : public SVGVisualProperty
    {
		double fMiterLimit{ 4.0 };
        
        //SVGStrokeMiterLimit() : SVGVisualProperty() {}
		SVGStrokeMiterLimit(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
		SVGStrokeMiterLimit(const SVGStrokeMiterLimit& other) :SVGVisualProperty(other) { fMiterLimit = other.fMiterLimit; }
        
        SVGStrokeMiterLimit& operator=(const SVGStrokeMiterLimit& rhs)
		{
			SVGVisualProperty::operator=(rhs);
			fMiterLimit = rhs.fMiterLimit;
			return *this;
        }

		void drawSelf(BLContext& ctx)
		{
			ctx.setStrokeMiterLimit(fMiterLimit);
		}
        
		void loadSelfFromChunk(const ByteSpan& inChunk)
		{
			fMiterLimit = toNumber(inChunk);
			fMiterLimit = clamp((float)fMiterLimit, 1.0f, 10.0f);
            
			set(true);
		}

		static std::shared_ptr<SVGStrokeMiterLimit> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
		{
			std::shared_ptr<SVGStrokeMiterLimit> sw = std::make_shared<SVGStrokeMiterLimit>(root);

			// If the chunk is empty, return immediately 
			if (inChunk)
				sw->loadFromChunk(inChunk);

			return sw;
		}

        // stroke-miterlimit
		static std::shared_ptr<SVGStrokeMiterLimit> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
		{
			return createFromChunk(root, name, elem.getAttribute(name));
		}
	
    };
    
    //=========================================================
    // SVGStrokeLineCap
    //=========================================================
    struct SVGStrokeLineCap : public SVGVisualProperty
    {
        BLStrokeCap fLineCap{ BL_STROKE_CAP_BUTT };


		SVGStrokeLineCap(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
		SVGStrokeLineCap(const SVGStrokeLineCap& other) :SVGVisualProperty(other)
		{
			fLineCap = other.fLineCap;
		}

		SVGStrokeLineCap& operator=(const SVGStrokeLineCap& rhs)
		{
			SVGVisualProperty::operator=(rhs);
			fLineCap = rhs.fLineCap;
			return *this;
		}

		void drawSelf(BLContext& ctx)
		{
            ctx.setStrokeCaps(fLineCap);
		}

		void loadSelfFromChunk(const ByteSpan& inChunk)
		{
            ByteSpan s = chunk_trim(inChunk, wspChars);

            set(true);
            
			if (s == "butt")
				fLineCap = BL_STROKE_CAP_BUTT;
			else if (s == "round")
				fLineCap = BL_STROKE_CAP_ROUND;
			else if (s == "square")
				fLineCap = BL_STROKE_CAP_SQUARE;
            else
			    set(false);
		}

		static std::shared_ptr<SVGStrokeLineCap> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
		{
			std::shared_ptr<SVGStrokeLineCap> stroke = std::make_shared<SVGStrokeLineCap>(root);

			// If the chunk is empty, return immediately 
			if (inChunk)
				stroke->loadFromChunk(inChunk);

			return stroke;
		}

		static std::shared_ptr<SVGStrokeLineCap> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem )
		{
			return createFromChunk(root, name, elem.getAttribute(name));
		}
	};

    //=========================================================
    // SVGStrokeLineJoin
	// A visual property to set the line join for a stroke
    //=========================================================
    struct SVGStrokeLineJoin : public SVGVisualProperty
    {
        BLStrokeJoin fLineJoin{ BL_STROKE_JOIN_MITER_BEVEL };

		SVGStrokeLineJoin(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
		SVGStrokeLineJoin(const SVGStrokeLineJoin& other) :SVGVisualProperty(other), fLineJoin(other.fLineJoin) {}  
        
        SVGStrokeLineJoin& operator=(const SVGStrokeLineJoin& rhs)
        {
			SVGVisualProperty::operator=(rhs);
			fLineJoin = rhs.fLineJoin;
			return *this;
        }
        
        void drawSelf(BLContext& ctx)
        {
			ctx.setStrokeJoin(fLineJoin);
        }
        
        void loadSelfFromChunk(const ByteSpan& inChunk)
        {
            ByteSpan s = chunk_trim(inChunk, wspChars);

            set(true);
            
			if (s == "miter")
				fLineJoin = BL_STROKE_JOIN_MITER_BEVEL;
			else if (s == "round")
				fLineJoin = BL_STROKE_JOIN_ROUND;
			else if (s == "bevel")
				fLineJoin = BL_STROKE_JOIN_BEVEL;
            //else if (s == "arcs")
			//	fLineJoin = SVG_JOIN_ARCS;
			else if (s == "miter-clip")
				fLineJoin = BL_STROKE_JOIN_MITER_CLIP;
			else
				set(false);

        }
        
		static std::shared_ptr<SVGStrokeLineJoin> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
		{
			std::shared_ptr<SVGStrokeLineJoin> stroke = std::make_shared<SVGStrokeLineJoin>(root);

			// If the chunk is empty, return immediately
			if (inChunk)
				stroke->loadFromChunk(inChunk);

			return stroke;
		}

        // stroke-linejoin
        static std::shared_ptr<SVGStrokeLineJoin> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
			return createFromChunk(root, name, elem.getAttribute(name));
        }
        
    };
    
}
namespace svg2b2d {
    
    //======================================================
    // SVGViewbox
    // A document may or may not have this property
    //======================================================
    
    struct SVGViewbox : public SVGVisualProperty
    {
        BLRect fRect{};


        SVGViewbox() :SVGVisualProperty(nullptr) {}
        //SVGViewbox():SVGVisualProperty(){}
		SVGViewbox(IMapSVGNodes* iMap):SVGVisualProperty(iMap){}
        SVGViewbox(const SVGViewbox& other)
            : SVGVisualProperty(other)
            ,fRect(other.fRect)
        {

        }

        SVGViewbox& operator=(const SVGViewbox& rhs)
        {
            SVGVisualProperty::operator=(rhs);
            fRect = rhs.fRect;

            return *this;
        }

        double x() const { return fRect.x; }
        double y() const { return fRect.y; }
        double width() const { return fRect.w; }
        double height() const {return fRect.h;}
        
        void loadSelfFromChunk(const ByteSpan& inChunk)
        {
			ByteSpan s = inChunk;
			ByteSpan numChunk;
            charset numDelims = wspChars+',';
                
            fRect.x = nextNumber(s, numDelims);
			fRect.y = nextNumber(s, numDelims);
			fRect.w = nextNumber(s, numDelims);
			fRect.h = nextNumber(s, numDelims);
        }

        static SVGViewbox createFromChunk(IMapSVGNodes* root, const ByteSpan& inChunk)
        {
            SVGViewbox vbox{};

            // If the chunk is empty, return immediately 
            if (!inChunk)
                return vbox;

            vbox.loadFromChunk(inChunk);

            return vbox;
        }

        // "viewBox"
        static SVGViewbox createFromXml(IMapSVGNodes* root, const XmlElement& elem, const std::string &name)
        {
            return createFromChunk(root, elem.getAttribute(name));
        }
    };
    


}

namespace svg2b2d
{


    static std::vector<BLPoint> parsePoints(const ByteSpan &inChunk)
	{
		std::vector<BLPoint> points;

		ByteSpan s = inChunk;
		charset numDelims = wspChars + ',';

		while (s)
		{
			BLPoint p;
			p.x = nextNumber(s, numDelims);
			p.y = nextNumber(s, numDelims);
			points.push_back(p);
		}

		return points;
	}
}

//================================================
// SVGTransform
// Transformation matrix
//================================================

namespace svg2b2d
{

    //
    // parsing transforms
    //
    static ByteSpan parseTransformArgs(const ByteSpan& inChunk, double* args, int maxNa, int& na)
    {
        na = 0;

        ByteSpan s = inChunk;

        // Skip past everything until we see a '('
		s = chunk_find_char(s, '(');

        // If we got to the end of the chunk, and did not see the '('
        // then just return
        if (!s)
            return s;

        // by the time we're here, we're sitting right on top of the 
        //'(', so skip past that to get to what should be the numbers
        s++;

        // We want to create a  chunk that contains all the numbers
        // without the closing ')', so create a chunk that just
        // represents that range.
        // Start the chunk at the current position
        // and expand it after we find the ')'
        ByteSpan item = s;
        item.fEnd = item.fStart;

        // scan until the closing ')'
        s = chunk_find_char(s, ')');


        // At this point, we're either sitting at the ')' or at the end
        // of the chunk.  If we're at the end of the chunk, then we
        // didn't find the closing ')', so just return
        if (!s)
            return s;

        // We found the closing ')', so if we use the current position
        // as the end (sitting on top of the ')', the item chunk will
        // perfectly represent the numbers we want to parse.
        item.fEnd = s.fStart;

        // Create a chunk that will represent a specific number to be parsed.
        ByteSpan numChunk{};

        // Move the source chunk cursor past the ')', so that whatever
        // needs to use it next is ready to go.
        s++;

        // Now we're ready to parse the individual numbers
		auto numDelims = wspChars + ',';
        
        while (item) {
            if (na >= maxNa)
                break;
            
			args[na++] = nextNumber(item, numDelims);
        }

        return s;
    }

    static ByteSpan parseMatrix(ByteSpan& inMatrix, BLMatrix2D &m)
    {
        ByteSpan s = inMatrix;
        m.reset();  // start with identity

        
        double t[6];    // storage for our 6 numbers
		int na = 0;     // Number of arguments parsed

        s = parseTransformArgs(s, t, 6, na);

        if (na != 6)
            return s;

		m.reset(t[0], t[1], t[2], t[3], t[4], t[5]);

        return s;
    }
    
    
    static ByteSpan parseTranslate(const ByteSpan& inChunk, BLMatrix2D& xform)
    {
        double args[2];
        int na = 0;
        ByteSpan s = inChunk;
        s = parseTransformArgs(s, args, 2, na);
        if (na == 1)
            args[1] = 0.0;

        xform.translate(args[0], args[1]);

        return s;
    }
    
    static ByteSpan parseScale(const ByteSpan& inChunk, BLMatrix2D& xform)
    {
        double args[2]{ 0 };
        int na = 0;
        ByteSpan s = inChunk;
        
        s = parseTransformArgs(s, args, 2, na);

        if (na == 1)
            args[1] = args[0];

        xform.scale(args[0], args[1]);

        return s;
    }
    
    
    static ByteSpan parseSkewX(const ByteSpan& inChunk, BLMatrix2D& xform)
    {
        double args[1];
        int na = 0;
        ByteSpan s = inChunk;
        s = parseTransformArgs(s, args, 1, na);

        xform.resetToSkewing(radians(args[0]), 0.0f);

        return s;
    }

    static ByteSpan parseSkewY(const ByteSpan& inChunk, BLMatrix2D& xform)
    {
        double args[1]{ 0 };
        int na = 0;
        ByteSpan s = inChunk;
        s = parseTransformArgs(s, args, 1, na);

        xform.resetToSkewing(0.0f, radians(args[0]));

        return s;
    }
    
    
    static ByteSpan parseRotate(const ByteSpan& inChunk, BLMatrix2D& xform)
    {
        double args[3]{ 0 };
        int na = 0;
        float m[6]{ 0 };
        float t[6]{ 0 };
        ByteSpan s = inChunk;

		s = parseTransformArgs(s, args, 3, na);

        if (na == 1)
            args[1] = args[2] = 0.0f;

		xform.rotate(radians(args[0]), args[1], args[2]);

        return  s;
    }


    struct SVGTransform : public SVGVisualProperty
    {
        BLMatrix2D fTransform{};

		SVGTransform(IMapSVGNodes* iMap) : SVGVisualProperty(iMap) {}
        SVGTransform(const SVGTransform& other)
            :SVGVisualProperty(other)
            ,fTransform(other.fTransform)
        {

        }

        SVGTransform& operator=(const SVGTransform& rhs)
        {
            SVGVisualProperty::operator=(rhs);
            fTransform = rhs.fTransform;

            return *this;
        }

        BLMatrix2D& getTransform() { return fTransform; }

		void loadSelfFromChunk(const ByteSpan& inChunk) override
		{
            ByteSpan s = inChunk;
            fTransform.reset();     // set to identity initially
            

            while (s)
            {
				s = chunk_skip_wsp(s);
                
                // Set out temp transform to the identity to start
                // so that if parsing goes wrong, we can still do
                // the multiply without worrying about messing things up
                // That means, the individula parsing functions need to not
                // partially mess up the transform if they fail.
                //svg_xformIdentity(t);

                BLMatrix2D tm{};
				tm.reset();

                if (chunk_starts_with_cstr(s, "matrix"))
                {
                    s = parseMatrix(s, tm);
                    fTransform = tm;
                    set(true);
                }
                else if (chunk_starts_with_cstr(s, "translate"))
                {
                    s = parseTranslate(s, tm);
                    fTransform.transform(tm);
                    set(true);
                }
                else if (chunk_starts_with_cstr(s, "scale"))
                {
                    s = parseScale(s, tm);
					fTransform.transform(tm);
                    set(true);
                }
                else if (chunk_starts_with_cstr(s, "rotate"))
                {
                    s = parseRotate(s, tm);
                    fTransform.transform(tm);
                    set(true);
                }
                else if (chunk_starts_with_cstr(s, "skewX"))
                {
                    s = parseSkewX(s,tm);
                    fTransform.transform(tm);
                    set(true);
                }
                else if (chunk_starts_with_cstr(s, "skewY"))
                {
                    s = parseSkewY(s,tm);
                    fTransform.transform(tm);
                    set(true);
                }
                else {
                    s++;
                }

            }

		}

        void drawSelf(BLContext& ctx) override
        {
			ctx.transform(fTransform);
        }

        static std::shared_ptr<SVGTransform> createFromChunk(IMapSVGNodes* root, const std::string& name, const ByteSpan& inChunk)
        {
			std::shared_ptr<SVGTransform> tform = std::make_shared<SVGTransform>(root);

            // If the chunk is empty, return immediately 
            if (inChunk)
                tform->loadFromChunk(inChunk);

            return tform;
        }

        // "transform"
        static std::shared_ptr<SVGTransform> createFromXml(IMapSVGNodes* root, const std::string& name, const XmlElement& elem)
        {
            return createFromChunk(root, name, elem.getAttribute(name));
        }
    };
    

}