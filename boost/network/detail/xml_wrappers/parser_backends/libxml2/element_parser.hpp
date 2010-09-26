//            Copyright (c) Glyn Matthews 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)


#ifndef __BOOST_NETWORK_DETAIL_XML_WRAPPERS_PARSER_BACKENDS_LIBXML2_ELEMENT_PARSER_INC__
# define __BOOST_NETWORK_DETAIL_XML_WRAPPERS_PARSER_BACKENDS_LIBXML2_ELEMENT_PARSER_INC__


# include <boost/network/traits/string.hpp>
# include <boost/network/detail/xml_wrappers/element.hpp>
# include <libxml/parser.h>
# include <libxml/tree.h>


namespace boost {
namespace network {
namespace detail {
template <
    class Tag
    >
class basic_libxml2_element_parser {

public:

    typedef typename string<Tag>::type string_type;

    typedef basic_element<Tag> element_type;

    basic_libxml2_element_parser() {
        std::memset(&handlers_, 0, sizeof(xmlSAXHandler));
        handlers_.startElement = start_element;
        handlers_.endElement = end_element;
        handlers_.characters = characters;
        depth_ = 0;
        element_ = 0;
        
        context_ = xmlCreatePushParserCtxt(&handlers_, 
                                           this, 0, 0, 0);
        // assert(!context_);
    }

    ~basic_libxml2_element_parser() {
        xmlFreeParserCtxt(context_);
    }

    bool feed(const string_type &chunk) {
        return feed(chunk, 0);
    }

    bool feed(const string_type &chunk, element_type *element) {
        element_ = element;
        return xmlParseChunk(context_, chunk.c_str(), chunk.size(), 0);
    }

private:

    static void set_name(element_type *element, const xmlChar *name) {
        const xmlChar *name_begin = name;
        const xmlChar *name_end = name_begin + xmlStrlen(name_begin);
        element->set_name(string_type(name_begin, name_end));
    }

    static void set_attributes(basic_element<Tag> *element, const xmlChar **attrs) {
        if (attrs) {
            for (int i = 0; attrs[i]; i += 2) {
                const xmlChar *key_begin = attrs[i];
                const xmlChar *key_end = key_begin + xmlStrlen(key_begin);

                const xmlChar *val_begin = attrs[i + 1];
                const xmlChar *val_end = val_begin + xmlStrlen(val_begin);

                element->set_attribute(string_type(key_begin, key_end),
                                       string_type(val_begin, val_end));
            }
        }
    }

    static void start_element(void *userdata,
                              const xmlChar *name,
                              const xmlChar **attrs) {
        basic_libxml2_element_parser<Tag> *parser
            = static_cast<basic_libxml2_element_parser<Tag> *>(userdata);

        if (parser->depth_ == 1) {
            set_name(parser->element_, name);
            set_attributes(parser->element_, attrs);
        }
        else if (parser->depth_ > 1) {
            element_type *child = new element_type;
            set_name(child, name);
            set_attributes(child, attrs);
            parser->element_->add_child(child);
        }
        
        ++parser->depth_;
    }

    static void end_element(void *userdata,
                            const xmlChar *name) {
        basic_libxml2_element_parser<Tag> *parser
            = static_cast<basic_libxml2_element_parser<Tag> *>(userdata);

        --parser->depth_;
    }

    static void characters(void *userdata,
                           const xmlChar *s,
                           int len) {
        basic_libxml2_element_parser<Tag> *parser
            = static_cast<basic_libxml2_element_parser<Tag> *>(userdata);

        parser->element_->add_child(
            new element_type(typename element_type::text(), string_type(s, s + len)));
    }
    
    xmlParserCtxtPtr context_;
    xmlSAXHandler handlers_;
    element_type *element_;
    int depth_;
    
};
} // namespace detail
} // namespace network
} // namespace boost


#endif // __BOOST_NETWORK_DETAIL_XML_WRAPPERS_PARSER_BACKENDS_LIBXML2_ELEMENT_PARSER_INC__
