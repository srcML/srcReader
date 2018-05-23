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

std::unordered_map<std::string, std::shared_ptr<srcml_node::srcml_namespace>> srcml_node::namespaces;

srcml_node::srcml_namespace::srcml_namespace(xmlNsPtr ns) : uri(), prefix() {

    if(!ns) return;

    if(ns->href)   uri = std::string((const char *)ns->href);
    if(ns->prefix) prefix = std::string((const char *)ns->prefix);
}

srcml_node::srcml_namespace::srcml_namespace(const srcml_namespace & ns) : uri(ns.uri), prefix(ns.prefix) {}

srcml_node::srcml_attribute::srcml_attribute(xmlAttrPtr attribute)
  : name((const char *)attribute->name),
    value(attribute->children && attribute->children->content ? 
          std::string((const char *)attribute->children->content) : boost::optional<std::string>()) {}

srcml_node::srcml_attribute::srcml_attribute(const srcml_attribute & attr) : name(attr.name), value(attr.value) {}

bool srcml_node::srcml_attribute::operator==(const srcml_attribute & that) const {

  if(name != that.name) return false;
  if(value == that.value && (!value || *value == *that.value)) return true;

  return false;

}

bool srcml_node::srcml_attribute::operator!=(const srcml_attribute & that) const {
  return !this->operator==(that);
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

std::shared_ptr<srcml_node::srcml_namespace> srcml_node::get_namespace(xmlNsPtr ns) {
  typedef  std::unordered_map<std::string, std::shared_ptr<srcml_namespace>>::const_iterator namespaces_citr;
  namespaces_citr citr = namespaces.find((const char *)ns->href);
  if(citr != namespaces.end()) return citr->second;

  namespaces_citr added_citr = namespaces.emplace(std::make_pair((const char *)ns->href, std::make_shared<srcml_namespace>(ns))).first;

  return citr->second;
}


srcml_node::srcml_node()
  : type(srcml_node_type::OTHER), name(), ns(), content(), ns_definition(), attributes(), is_empty(false), extra(0) {}

srcml_node::srcml_node(const xmlNode & node) 
  : type(xml_type2srcml_type(node.type)), name(), ns(), content(), ns_definition(), attributes(), is_empty(node.extra), extra(node.extra) {

  name = std::string((const char *)node.name);

  if(node.content)
    content = std::string((const char *)node.content);

  if(node.ns) ns = get_namespace(node.ns);

  xmlNsPtr node_ns = node.nsDef;
  while(node_ns) {
    ns_definition.emplace_back(get_namespace(node_ns));
    node_ns = node_ns->next;
  }

  xmlAttrPtr attribute = node.properties;
  while (attribute) {
    attributes.emplace(std::make_pair(std::string((const char *)attribute->name), srcml_attribute(attribute)));
    attribute = attribute->next;
  }

}

srcml_node::srcml_node(const srcml_node & node) 
  : type(node.type), name(node.name), ns(node.ns), content(node.content), ns_definition(node.ns_definition),
    attributes(node.attributes), is_empty(node.is_empty), extra(node.extra) {}

srcml_node::srcml_node(const std::string & text)
  : type(srcml_node_type::TEXT), name(), content(text), is_empty(true), extra(0) {}


srcml_node::~srcml_node() {}

bool srcml_node::operator==(const srcml_node & node) const {
  return type == node.type && name == node.name && content == node.content;
}

bool srcml_node::operator!=(const srcml_node & node) const {
  return !operator==(node);
}

bool srcml_node::is_text() const {

  return type == srcml_node_type::TEXT;
}
