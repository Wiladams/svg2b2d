#pragma once

#include "xmlscan.h"

namespace ndt_debug {
    using namespace svg2b2d;

    std::map<int, std::string> elemTypeNames = {
     {svg2b2d::XML_ELEMENT_TYPE_INVALID, "INVALID"}
    ,{svg2b2d::XML_ELEMENT_TYPE_CONTENT, "CONTENT"}
    ,{svg2b2d::XML_ELEMENT_TYPE_SELF_CLOSING, "SELF_CLOSING"}
    ,{svg2b2d::XML_ELEMENT_TYPE_START_TAG, "START_TAG"}
    ,{svg2b2d::XML_ELEMENT_TYPE_END_TAG, "END_TAG"}
    ,{svg2b2d::XML_ELEMENT_TYPE_COMMENT, "COMMENT"}
    ,{svg2b2d::XML_ELEMENT_TYPE_PROCESSING_INSTRUCTION, "PROCESSING_INSTRUCTION"}
    ,{svg2b2d::XML_ELEMENT_TYPE_CDATA, "CDATA"}
    ,{svg2b2d::XML_ELEMENT_TYPE_XMLDECL, "XMLDECL"}
    ,{svg2b2d::XML_ELEMENT_TYPE_DOCTYPE, "DOCTYPE"}
    };

    void printXmlElement(const svg2b2d::XmlElement& elem)
    {
        if (elem.kind() == XML_ELEMENT_TYPE_INVALID)
            return;

        switch (elem.kind())
        {
        case svg2b2d::XML_ELEMENT_TYPE_CONTENT:
        case svg2b2d::XML_ELEMENT_TYPE_COMMENT:
        case svg2b2d::XML_ELEMENT_TYPE_PROCESSING_INSTRUCTION:
        case svg2b2d::XML_ELEMENT_TYPE_DOCTYPE:
            printf("%s: \n", elemTypeNames[elem.kind()].c_str());
            printChunk(elem.data());
            break;

        //case svg2b2d::XML_ELEMENT_TYPE_DOCTYPE:
        //    printf("%s: \n", elemTypeNames[elem.kind()].c_str());
        //    break;
            
        case svg2b2d::XML_ELEMENT_TYPE_START_TAG:
            printf("START_TAG: [%s]\n", elem.name().c_str());
            break;

        case svg2b2d::XML_ELEMENT_TYPE_SELF_CLOSING:
            printf("SELF_CLOSING: [%s]\n", elem.name().c_str());
            break;

        case svg2b2d::XML_ELEMENT_TYPE_END_TAG:
            printf("END_TAG: [%s]\n", elem.name().c_str());
            break;


                
        default:
            printf("NYI: %s\n", elemTypeNames[elem.kind()].c_str());
            printChunk(elem.data());
            break;
        }

        for (auto& attr : elem.attributes())
        {
            printf("    %s: ", attr.first.c_str());
            printChunk(attr.second);
        }
    }
}
