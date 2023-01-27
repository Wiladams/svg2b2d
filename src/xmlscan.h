#pragma once

#include "bspanutil.h"


#include <string>
#include <vector>
#include <map>
#include <tuple>

//
// This file represents a very small, fast, simple XML scanner
// The purpose is to break a chunk of XML down into component parts, that higher
// level code can then use to do whatever it wants.
// 
// You can construct an iterator, and use that to scan through the XML
// using a 'pull model'.
// 
// One key aspect of the design is that it operates on a span of memory.  It does
// not deal with files, or streams, or anything high level like that, just a ByteSpan.
// It does not alter the span, just reads bytes from it, and returns spans in 
// responses.
//
// The fundamental unit is the XmlElement, which encapsulates a single XML element
// and its attributes.
//
// The element contains individual members for
//  kind - content, self-closing, start-tag, end-tag, comment, processing-instruction
//  name - the name of the element, if opening or closing tag
//  attributes - a map of attribute names to attribute values.  Values are still in raw form
//  data - the raw data of the element.  
// The starting name has been removed, to be turned into the name
// 
// The XmlElementIterator is used to iterate over the elements in a chunk of memory.
//
// References:
// https://dvcs.w3.org/hg/microxml/raw-file/tip/spec/microxml.html
// https://www.w3.org/TR/REC-xml/
//

namespace svg2b2d {

    enum XML_ELEMENT_TYPE {
        XML_ELEMENT_TYPE_INVALID = 0
		, XML_ELEMENT_TYPE_XMLDECL                  // An XML declaration, like <?xml version="1.0" encoding="UTF-8"?>
        , XML_ELEMENT_TYPE_CONTENT                  // Content, like <foo>bar</foo>, the 'bar' is content
        , XML_ELEMENT_TYPE_SELF_CLOSING             // A self-closing tag, like <foo/>
        , XML_ELEMENT_TYPE_START_TAG                // A start tag, like <foo>
        , XML_ELEMENT_TYPE_END_TAG                  // An end tag, like </foo>
        , XML_ELEMENT_TYPE_COMMENT                  // A comment, like <!-- foo -->
        , XML_ELEMENT_TYPE_PROCESSING_INSTRUCTION   // A processing instruction, like <?foo bar?>
        , XML_ELEMENT_TYPE_CDATA                    // A CDATA section, like <![CDATA[ foo ]]>
        , XML_ELEMENT_TYPE_DOCTYPE                  // A DOCTYPE section, like <!DOCTYPE foo>
    };

	struct XmlName {
        ByteSpan fNamespace{};
        ByteSpan fName{};

        XmlName() = default;
        
        XmlName(const ByteSpan& inChunk)
        {
            reset(inChunk);
        }

        XmlName(const XmlName &other):fNamespace(other.fNamespace), fName(other.fName){}
        
        XmlName& operator =(const XmlName& rhs)
        {
            fNamespace = rhs.fNamespace;
            fName = rhs.fName;
            return *this;
        }
        
        XmlName & operator=(const ByteSpan &inChunk)
        {
            reset(inChunk);
            return *this;
        }
        
		// Implement for std::map, and ordering in general
		bool operator < (const XmlName& rhs) const
		{
			size_t maxnsbytes = std::min(fNamespace.size(), rhs.fNamespace.size());
			size_t maxnamebytes = std::min(fName.size(), rhs.fName.size());
            
			return (memcmp(fNamespace.begin(), rhs.fNamespace.begin(), maxnsbytes)<=0)  && (memcmp(fName.begin(), rhs.fName.begin(), maxnamebytes) < 0);
		}
        
        // Allows setting the name after it's been created
        // BUGBUG - maybe an operator= would be better?
        XmlName& reset(const ByteSpan& inChunk)
        {
            fName = inChunk;
            fNamespace = chunk_token(fName, charset(':'));
            if (chunk_size(fName)<1)
            {
                fName = fNamespace;
                fNamespace = {};
            }
            return *this;
        }
        
		ByteSpan name() const { return fName; }
		ByteSpan ns() const { return fNamespace; }
	};
    
    // Representation of an xml element
    // The xml iterator will generate these
    struct XmlElement
    {
    private:
        int fElementKind{ XML_ELEMENT_TYPE_INVALID };
        ByteSpan fData{};

        XmlName fXmlName{};
        std::string fName{};
        std::map<std::string, ByteSpan> fAttributes{};

    public:
        XmlElement() {}
        XmlElement(int kind, const ByteSpan& data, bool autoScanAttr = false)
            :fElementKind(kind)
            , fData(data)
        {
            reset(kind, data, autoScanAttr);
        }

		void reset(int kind, const ByteSpan& data, bool autoScanAttr = false)
		{
            clear();

            fElementKind = kind;
            fData = data;

            if ((fElementKind == XML_ELEMENT_TYPE_START_TAG) ||
                (fElementKind == XML_ELEMENT_TYPE_SELF_CLOSING) ||
                (fElementKind == XML_ELEMENT_TYPE_END_TAG))
            {
                scanTagName();

                if (autoScanAttr) {
                    if (fElementKind != XML_ELEMENT_TYPE_END_TAG)
                        scanAttributes();
                }
            }
		}
        
		// Clear this element to a default state
        void clear() {
			fElementKind = XML_ELEMENT_TYPE_INVALID;
			fData = {};
			fName.clear();
			fAttributes.clear();
		}
        
        // determines whether the element is currently empty
        bool empty() const { return fElementKind == XML_ELEMENT_TYPE_INVALID; }

        explicit operator bool() const { return !empty(); }

        // Returning information about the element
        const std::map<std::string, ByteSpan>& attributes() const { return fAttributes; }
        
        const std::string& name() const { return fName; }
		void setName(const std::string& name) { fName = name; }
        
        int kind() const { return fElementKind; }
		void kind(int kind) { fElementKind = kind; }
        
        const ByteSpan& data() const { return fData; }

		// Convenience for what kind of tag it is
        bool isStart() const { return (fElementKind == XML_ELEMENT_TYPE_START_TAG); }
		bool isSelfClosing() const { return fElementKind == XML_ELEMENT_TYPE_SELF_CLOSING; }
		bool isEnd() const { return fElementKind == XML_ELEMENT_TYPE_END_TAG; }
		bool isComment() const { return fElementKind == XML_ELEMENT_TYPE_COMMENT; }
		bool isProcessingInstruction() const { return fElementKind == XML_ELEMENT_TYPE_PROCESSING_INSTRUCTION; }
        bool isContent() const { return fElementKind == XML_ELEMENT_TYPE_CONTENT; }
		bool isCData() const { return fElementKind == XML_ELEMENT_TYPE_CDATA; }
		bool isDoctype() const { return fElementKind == XML_ELEMENT_TYPE_DOCTYPE; }

        
        void addAttribute(std::string& name, const ByteSpan& valueChunk)
        {
            fAttributes[name] = valueChunk;
        }

        ByteSpan getAttribute(const std::string &name) const
		{
			auto it = fAttributes.find(name);
			if (it != fAttributes.end())
				return it->second;
			else
                return ByteSpan{};
		}
        
    private:
        //
        // Parse an XML element
        // We should be sitting on the first character of the element tag after the '<'
        // There are several things that need to happen here
        // 1) Scan the element name
        // 2) Scan the attributes, creating key/value pairs
        // 3) Figure out if this is a self closing element

        // 
        // We do NOT scan the content of the element here, that happens
        // outside this routine.  We only deal with what comes up the the closing '>'
        //
        void setTagName(const ByteSpan& inChunk)
        {
            fXmlName.reset(inChunk);
            fName = toString(fXmlName.name());
        }
        
        void scanTagName()
        {
            ByteSpan s = fData;
            bool start = false;
            bool end = false;

            // If the chunk is empty, just return
            if (!s)
                return;

            // Check if the tag is end tag
            if (*s == '/')
            {
                s++;
                end = true;
            }
            else {
                start = true;
            }

            // Get tag name
            ByteSpan tagName = s;
            tagName.fEnd = s.fStart;

            while (s && !wspChars[*s])
                s++;

            tagName.fEnd = s.fStart;
            setTagName(tagName);


            fData = s;
        }

        public:
        //
        // scanAttributes
        // Scans the fData member looking for attribute key/value pairs
        // It will add to the member fAttributes these pairs, without further processing.
        // This should be called after scanTagName(), because we want to be positioned
        // on the first key/value pair. 
        //
        int scanAttributes()
        {

            int nattr = 0;
            bool start = false;
            bool end = false;
            uint8_t quote{};
            ByteSpan s = fData;


            // Get the attribute key/value pairs for the element
            while (s && !end)
            {
                uint8_t* beginattrValue = nullptr;
                uint8_t* endattrValue = nullptr;


                // Skip white space before the attrib name
                s = chunk_ltrim(s, wspChars);

                if (!s)
                    break;

                if (*s == '/') {
                    end = true;
                    break;
                }

                // Find end of the attrib name.
                //static charset equalChars("=");
                auto attrNameChunk = chunk_token(s, "=");
                attrNameChunk = chunk_trim(attrNameChunk, wspChars);    // trim whitespace on both ends

                std::string attrName = std::string(attrNameChunk.fStart, attrNameChunk.fEnd);

                // Skip stuff past '=' until the beginning of the value.
                while (s && (*s != '\"') && (*s != '\''))
                    s++;

                // If we've reached end of span, bail out
                if (!s)
                    break;

                // capture the quote character
                // Store value and find the end of it.
                quote = *s;

				s++;    // move past the quote character
                beginattrValue = (uint8_t*)s.fStart;    // Mark the beginning of the attribute content

                // Skip until we find the matching closing quote
                while (s && *s != quote)
                    s++;

                if (s)
                {
                    endattrValue = (uint8_t*)s.fStart;  // Mark the ending of the attribute content
                    s++;
                }

                // Store only well formed attributes
                ByteSpan attrValue = { beginattrValue, endattrValue };

                addAttribute(attrName, attrValue);

                nattr++;
            }

            return nattr;
        }
    };


    


    // XmlElementIterator
    // scans XML generating a sequence of XmlElements
    // This is a forward only non-writeable iterator
    // 
	// Usage:
    //   XmlElementIterator iter = XmlElementIterator(xmlChunk);
    //   XmlElement elem;
    //   do {
	//      elem = iter.next();
    //      if (elem)
	//          processElement(elem);
	//   } while (elem);
    //
    //
    // Language syntax: https://www.w3.org/TR/REC-xml/
    // doctypedecl	   ::=   	'<!DOCTYPE' S Name (S ExternalID)? S? ('[' intSubset ']' S?)? '>'
    // 
    // DeclSep	       ::=   	PEReference | S
    // intSubset	   ::=   	(markupdecl | DeclSep)*
    // markupdecl	   ::=   	elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment
    // ExternalID	   ::=   	'SYSTEM' S SystemLiteral
    // 		                    | 'PUBLIC' S PubidLiteral S SystemLiteral
    // NDataDecl	   ::=   	S 'NDATA' S Name

    struct XmlElementIterator {
    private:
        // XML Iterator States
        enum XML_ITERATOR_STATE {
            XML_ITERATOR_STATE_CONTENT = 0
            , XML_ITERATOR_STATE_START_TAG

        };
        
        // What state the iterator is in
        int fState{ XML_ITERATOR_STATE_CONTENT };
        svg2b2d::ByteSpan fSource{};
        svg2b2d::ByteSpan mark{};

        XmlElement fCurrentElement{};
        
    public:
        XmlElementIterator(const svg2b2d::ByteSpan& inChunk)
        {
            fSource = inChunk;
            mark = inChunk;

            fState = XML_ITERATOR_STATE_CONTENT;
            
            next();
        }

		explicit operator bool() { return !fCurrentElement.empty(); }
        
        // These operators make it operate like an iterator
        const XmlElement& operator*() const { return fCurrentElement; }
        const XmlElement* operator->() const { return &fCurrentElement; }

        XmlElementIterator& operator++() { next(); return *this; }
        XmlElementIterator& operator++(int) { next(); return *this; }
        
        // Reset the iterator to a known state with data
        void reset(const svg2b2d::ByteSpan& inChunk, int st)
        {
            fSource = inChunk;
            mark = inChunk;

            fState = st;
        }

        ByteSpan readTag()
        {
            ByteSpan elementChunk = fSource;
            elementChunk.fEnd = fSource.fStart;
            
            while (fSource && *fSource != '>')
                fSource++;

            elementChunk.fEnd = fSource.fStart;
            elementChunk = chunk_rtrim(elementChunk, wspChars);
            
            // Get past the '>' if it was there
            fSource++;
            
            return elementChunk;
        }
        
        // readDoctype
		// Reads the doctype chunk, and returns it as a ByteSpan
        // fSource is currently sitting at the beginning of !DOCTYPE
        // Note: 
        
        ByteSpan readDoctype()
        {

            // skip past the !DOCTYPE to the first whitespace character
			while (fSource && !wspChars[*fSource])
				fSource++;
            
			// Skip past the whitespace
            // to get to the beginning of things
			fSource = chunk_ltrim(fSource, wspChars);

            
            // Mark the beginning of the "content" we might return
            ByteSpan elementChunk = fSource;
            elementChunk.fEnd = fSource.fStart;

            // To get to the end, we're looking for '[]' or just '>'
            auto foundChar = chunk_find_char(fSource, '[');
            if (foundChar)
            {
                fSource = foundChar;
                foundChar = chunk_find_char(foundChar, ']');
                if (foundChar)
                {
                    fSource = foundChar;
                    fSource++;
                }
                elementChunk.fEnd = fSource.fStart;
            }
            
            // skip whitespace?
            // search for closing '>'
            foundChar = chunk_find_char(fSource, '>');
            if (foundChar)
            {
                fSource = foundChar;
                elementChunk.fEnd = fSource.fStart;
                fSource++;
            }
            
            return elementChunk;
        }
        
        
        // Simple routine to scan XML content
        // the input 's' is a chunk representing the xml to 
        // be scanned.
        // The input chunk will be altered in the process so it
        // can be used in a subsequent call to continue scanning where
        // it left off.
        bool next()
        {
            while (fSource)
            {
                switch (fState)
                {
                case XML_ITERATOR_STATE_CONTENT: {

                    if (*fSource == '<')
                    {
                        // Change state to beginning of start tag
                        // for next turn through iteration
                        fState = XML_ITERATOR_STATE_START_TAG;

                        if (fSource != mark)
                        {
                            // Encapsulate the content in a chunk
                            svg2b2d::ByteSpan content = { mark.fStart, fSource.fStart };

                            // collapse whitespace
							// if the content is all whitespace
                            // don't return anything
							content = chunk_trim(content, wspChars);
                            if (content)
                            {
                                // Set the state for next iteration
                                fSource++;
                                mark = fSource;
                                fCurrentElement.reset(XML_ELEMENT_TYPE_CONTENT, content);
                                
                                return true;
                            }
                        }

                        fSource++;
                        mark = fSource;
                    }
                    else {
                        fSource++;
                    }

                }
                break;

                case XML_ITERATOR_STATE_START_TAG: {
                    // Create a chunk that encapsulates the element tag 
                    // up to, but not including, the '>' character
                    ByteSpan elementChunk = fSource;
                    elementChunk.fEnd = fSource.fStart;
                    int kind = XML_ELEMENT_TYPE_START_TAG;
                    
                    if (chunk_starts_with_cstr(fSource, "?xml"))
                    {
						kind = XML_ELEMENT_TYPE_XMLDECL;
                        elementChunk = readTag();
                    } 
                    else if (chunk_starts_with_cstr(fSource, "?"))
                    {
                        kind = XML_ELEMENT_TYPE_PROCESSING_INSTRUCTION;
                        elementChunk = readTag();
                    }
                    else if (chunk_starts_with_cstr(fSource, "!DOCTYPE"))
                    {
                        kind = XML_ELEMENT_TYPE_DOCTYPE;
                        elementChunk = readDoctype();
                    }
                    else if (chunk_starts_with_cstr(fSource, "!--"))
                    {
						kind = XML_ELEMENT_TYPE_COMMENT;
                        elementChunk = readTag();
                    }
                    else if (chunk_starts_with_cstr(fSource, "![CDATA["))
                    {
                        kind = XML_ELEMENT_TYPE_CDATA;
                        elementChunk = readTag();
                    }
					else if (chunk_starts_with_cstr(fSource, "/"))
					{
						kind = XML_ELEMENT_TYPE_END_TAG;
						elementChunk = readTag();
					}
					else {
						elementChunk = readTag();
                        if (chunk_ends_with_char(elementChunk, '/'))
                            kind = XML_ELEMENT_TYPE_SELF_CLOSING;
					}
                    
                    fState = XML_ITERATOR_STATE_CONTENT;

                    mark = fSource;

					fCurrentElement.reset(kind, elementChunk, true);

                    return true;
                }
                break;

                default:
                    fSource++;
                    break;

                }
            }

            fCurrentElement.clear();
            return false;
        } // end of next()
    };
}



