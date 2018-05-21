/**
 * @file src_author.hpp
 *
 * This is a generic C++ header template.
 *
 * @author Michael John Decker, Ph.D. <mdecke@bsgu.edu>
 */

#ifndef INCLUDED_SRCE_AUTHOR_HPP
#define INCLUDED_SRCE_AUTHOR_HPP

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

#include <stack>

class src_author {

private:

    xmlTextReaderPtr author_reader;
    xmlTextReaderPtr next_reader;
    xmlTextWriterPtr merged_writer;

    std::string author;

    static const std::string srcdiff_namespace = "http://www.srcML.org/srcDiff";

    void cleanup(const std::string & error_message = std::string()) {

        if(author_reader) xmlTextReaderClose(author_reader);
        if(next_reader) xmlTextReaderClose(next_reader);
        if(merged_reader) xmlTextWriterClose(merged_writer);

        if(error_message != std:string()) throw error_message;

    }

    xmlNodePtr get_next_node(xmlTextReaderPtr reader) {

        int success = xmlTextReaderRead(reader);
        if(success == -1) cleanup("Error reading file");
        if(!success) return nullptr;

        return xmlTextReaderCurrentNode(reader);

    }

    void write_node(xmlNodePtr node) {

        switch((xmlReaderTypes)XMLnode->type) {

            case XML_READER_TYPE_ELEMENT: {

                xmlTextWriterStartElementNS(merged_writer, node->ns->prefix, node->ns->href, node->name);
                // attributes?
            }

            case XML_READER_TYPE_END_ELEMENT: {
                xmlTextWriterEndElement(merged_writer);
            }

            case XML_READER_TYPE_TEXT:
            case XML_READER_TYPE_WHITESPACE:
            case XML_READER_TYPE_SIGNIFICANT_WHITESPACE: {

                char * text = (char *)node->content;
                for(char * pos = text; *pos; ++pos) {

                    if (*pos != '"')
                        continue;

                    *pos = 0;
                    if (xmlTextWriterWriteString(out.getWriter(), (xmlChar *)text) == -1)
                        cleanup("Error");

                    *pos = '\"';
                    if ( xmlTextWriterWriteRaw(out.getWriter(), (xmlChar *)"\"") == -1)
                        cleanup("Error");

                    text = pos + 1;
                }

                int ret = xmlTextWriterWriteString(out.getWriter(), (xmlChar *)text);
                if(ret == -1) cleanup("error");

            }

        }

    }

    static bool xmlstr_compare(xmlChar * xml_str, const std::string & str) {

        return std::string((const char *)xml_str) == str;

    }

public:
    src_author(const std::string & author_srcml,
               const std::string & next_srcml,
               const std::string & merged_srcml,
               const std::string & author) 
        : author_reader(nullptr), next_reader(nullptr), merged_writer(nullptr),
          author(author) {

        author_reader = xmlNewTextReaderFilename(author_srcml.c_str());
        if(!author_reader) cleanup("Error with author file")

        next_reader = xmlNewTextReaderFilename(next_srcml.c_str());
        if(!next_reader) cleanup("Error with next file");

        merged_writer = xmlNewTextWriterFilename(merged_writer.c_str());
        if(!merged_writer) cleanup("Error with merged file")

    }

    ~src_author() {
        cleanup();
    }

    void merge_next() {

        enum srcdiff_op { COMMON, DELETE, INSERT };

        xmlNodePtr author_node = nullptr;
        xmlNodePtr node = nullptr;

        std::stack<srcdiff_op> op_stack;
        while((node = get_next_node(next_reader))) {


            if(node->ns && node->ns->href && xmlstr_compare(node->ns->href, srcdiff_namespace)) {

                if((xmlReaderTypes)node->type == XML_READER_TYPE_END_ELEMENT) {
                    op_stack.pop();
                    continue;
                }

                if(xmlstr_compare(node->name, "common"))
                    op_stack.push(srcdiff_op::COMMON);
                else if(xmlstr_compare(node->name, "delete"))
                    op_stack.push(srcdiff_op::DELETE);
                else if(xmlstr_compare(node->name, "insert"))
                    op_stack.push(srcdiff_op::INSERT);

                continue;
            }



        }

};

#endif