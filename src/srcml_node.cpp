/*
  srcml_node.cpp

  Copyright (C) 2018 srcML, LLC. (www.srcML.org)

  This file is part of a translator from source code to srcMLIdentify

  srcIdentity is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  srcMLIdentify is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the srcML translator; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <srcml_node.hpp>

#include <srcml.h>

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>


#ifdef __MINGW32__
#include <mingw32.hpp>
#endif

srcml_node::srcml_ns::srcml_ns(const srcml_ns & ns) : href(ns.href), prefix(ns.prefix) {}

bool srcml_node::srcml_attr::operator==(const srcml_attr & attr) const {

  if(name != attr.name) return false;

  if(value == attr.value && (!value || *value == *attr.value)) return true;

  return false;


}

bool srcml_node::srcml_attr::operator!=(const srcml_attr & attr) const {

  return !this->operator==(attr);


}

srcml_node::srcml_node_type xml_type2srcml_type(xmlElementType type) {
  static std::unordered_map<unsigned int, srcml_node::srcml_node_type> type_map = {

    { XML_READER_TYPE_ELEMENT, srcml_node::srcml_node_type::START },
    { XML_READER_TYPE_END_ELEMENT, srcml_node::srcml_node_type::END },
    { XML_READER_TYPE_TEXT, srcml_node::srcml_node_type::TEXT },
    { XML_READER_TYPE_SIGNIFICANT_WHITESPACE, srcml_node::srcml_node_type::TEXT },

  };

  try {
    return type_map.at((unsigned int)type);
  } catch(const std::out_of_range & error) {
    return srcml_node::srcml_node_type::OTHER;
  }

}

srcml_node::srcml_node(const xmlNode & node) 
  : type(xml_type2srcml_type(node.type)), name(), ns(), ns_def(), is_empty(node.extra), extra(node.extra) {

  name = std::string((const char *)node.name);

  if(content)
    content = std::string((const char *)node.content);

  if(node.ns) {

    if(node.ns->href)
      ns.href = std::string((const char *)node.ns->href);

    if(node.ns->prefix)
      ns.prefix = std::string((const char *)node.ns->prefix);
  }

  xmlNsPtr node_ns = node.nsDef;
  while(node_ns) {
    ns_def.emplace_back(std::string((const char *)node_ns->prefix), std::string((const char *)node_ns->href));
    node_ns = node_ns->next;
  }

  xmlAttrPtr attribute = node.properties;
  while (attribute) {
    properties.emplace_back(std::string((const char *)attribute->name), std::string((const char *)attribute->children->content));
    attribute = attribute->next;
  }

}

srcml_node::srcml_node(const srcml_node & node) 
  : type(node.type), name(node.name), content(node.content), is_empty(node.is_empty), extra(node.extra) {

  ns = node.ns;

  for(const srcml_attr & attr : node.properties)
    properties.push_back(attr);

}

srcml_node::srcml_node(const std::string & text)
  : type(srcml_node_type::TEXT), name(), content(text), is_empty(true), extra(0) {}


srcml_node::~srcml_node() {}

bool srcml_node::operator==(const srcml_node & node) const {

  return type == node.type
    && name == node.name
    && (((xmlReaderTypes)type != XML_READER_TYPE_TEXT && (xmlReaderTypes)type != XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
      || (content == node.content && (!content || *content == *node.content)));
}

bool srcml_node::is_text() const {

  return type == srcml_node_type::TEXT;
}
