/*
  srcml_reader.cpp

  Copyright (C) 2018 srcML, LLC. (www.srcML.org)

  This file is part of a translator from source code to srcReader

  srcReader is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  srcReader is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the srcML translator; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <algorithm>
#include <srcml_reader.hpp>
#include <vector>
#include <string>

#include <iostream>

class srcml_reader_error : public std::runtime_error {
public:
  srcml_reader_error(const std::string & what_arg) : std::runtime_error(what_arg) {}

};

void srcml_reader::cleanup() {

  if(reader) {
    xmlTextReaderClose(reader);
    reader = nullptr;
  }

}

srcml_reader::srcml_reader(const std::string & filename) 
  : reader(nullptr), offset(std::string::npos), saved_node(), issue_end_tag(false), current_node(),
    is_eof(false), iterator() {

  reader = xmlNewTextReaderFilename(filename.c_str());
  if(!reader) {
    cleanup();
    throw srcml_reader_error("Error openining: " + filename);
  }

}

srcml_reader::~srcml_reader() {
  cleanup();
}

const std::stack<std::string> & srcml_reader::get_element_stack() const {
  return element_stack;
}

static std::string::size_type find_count(const std::string & str, std::string::size_type start) {

    bool is_space = std::isspace(str[start]);
    std::string::size_type size = str.size();
    std::string::size_type end = start + 1;
    while(end < size && std::isspace(str[end]) == is_space) {
      ++end;
    }

    if(end >= size) return std::string::npos;
    return end - start;
}

void srcml_reader::update_current_text_node() {

    std::string::size_type count = find_count(*saved_node->content, offset);
    current_node = std::make_unique<srcml_node>(saved_node->content->substr(offset, count));

    if(count != std::string::npos) {
      offset += count;
    } else {
      offset = std::string::npos;
    }
}

bool srcml_reader::read() {
  if(is_eof) return false;

  if(offset != std::string::npos && current_node && current_node->is_text()) {
    update_current_text_node();
    return true;

  } else if(issue_end_tag) {
    issue_end_tag = false;

    current_node = std::make_unique<srcml_node>(*current_node);
    current_node->type = srcml_node::srcml_node_type::END;
    current_node->attributes.clear();
    current_node->ns_definition.clear();

    if(element_stack.size() > 1) {
      element_stack.pop();
    } 

    return true;

  }

  int success = xmlTextReaderRead(reader);
  if(success == -1) throw srcml_reader_error("Error reading file");
  if(!success) {
    is_eof = true;
    current_node = std::make_unique<srcml_node>(srcml_node());
    return false;
  }

  xmlNodePtr node = xmlTextReaderCurrentNode(reader);
  if(!node) throw srcml_reader_error("Error getting current node");

  int type = xmlTextReaderNodeType(reader);
  if(type == -1) srcml_reader_error("Error getting node type");

  srcml_node * temp_node = nullptr;
  try {
    temp_node = new srcml_node(*node, (xmlElementType)type);
  } catch(const std::bad_alloc & memory_error) {
    throw srcml_reader_error("Memory error getting node");
  }

  if(temp_node->is_text()) {
    offset = 0;
    saved_node = std::unique_ptr<srcml_node>(temp_node);
    update_current_text_node();
  } else {
    current_node = std::unique_ptr<srcml_node>(temp_node);
    if(current_node->is_empty()) {
      issue_end_tag = true;
      current_node->empty = false;
    }

    if(current_node->is_start()) {
      element_stack.push(current_node->full_name());
    } else if(current_node->is_end()){
      element_stack.pop();
    }
  }
  return true;
}

srcml_reader::srcml_reader_iterator srcml_reader::begin() {

  if(!iterator.reader) {
    read();
    iterator.reader = this;
  }

  return iterator;
}

srcml_reader::srcml_reader_iterator srcml_reader::end() {
  return srcml_reader_iterator();
}

xmlDocPtr srcml_reader::get_current_doc() const {
  xmlDocPtr doc = xmlTextReaderCurrentDoc(reader);
  if(doc == nullptr) {
    throw srcml_reader_error("Error getting current doc");
  }
  return doc;
}

xmlNodePtr srcml_reader::expand_current_node() const {
  xmlNodePtr subtree = xmlTextReaderExpand(reader);
  if(subtree == nullptr) {
    throw srcml_reader_error("Error expanding current node");
  }
  return subtree;

}


srcml_reader::operator bool() const {
  return *current_node != srcml_node();
}

srcml_reader::srcml_reader_iterator::srcml_reader_iterator(srcml_reader * reader)
  : reader(reader) {}

const srcml_node & srcml_reader::srcml_reader_iterator::operator*() const {
  return *reader->current_node; 
}

srcml_node & srcml_reader::srcml_reader_iterator::operator*() {
  return *reader->current_node; 
}

const srcml_node * srcml_reader::srcml_reader_iterator::operator->() const {
  return reader->current_node.get(); 
}

srcml_node * srcml_reader::srcml_reader_iterator::operator->() {
  return reader->current_node.get(); 
}

const srcml_node & srcml_reader::srcml_reader_iterator::operator++() {

  reader->read();
  return *reader->current_node;

}

srcml_node srcml_reader::srcml_reader_iterator::operator++(int) {

  srcml_node node = *reader->current_node;
  reader->read();
  return node;

}

bool srcml_reader::srcml_reader_iterator::operator!=(const srcml_reader_iterator & that) const {
  return *reader;
}

static void collect_text(xmlNodePtr node, std::string &out) {
  for (xmlNodePtr cur = node; cur; cur = cur->next) {
    if (cur->type == XML_TEXT_NODE || cur->type == XML_CDATA_SECTION_NODE) {
      if (cur->content)
        out.append(reinterpret_cast<const char *>(cur->content));
    }

    if (cur->children)
      collect_text(cur->children, out);
  }
}


std::string srcml_reader::get_current_inner_text() const {
  xmlNodePtr subtree = xmlTextReaderExpand(reader);
  if (!subtree)
    throw srcml_reader_error("Error expanding current node");

  std::string result;
  collect_text(subtree->children, result); // children only = "inside the tag"
  return result;
}

std::string srcml_reader::build_segment(xmlNodePtr node) const {
  const std::string qname = node_qname(node);
  const std::string predicate = semantic_predicate(node);
  if (!predicate.empty()) {
    return qname + predicate;
  }
  const std::size_t pos = sibling_position(node);
  if (pos > 1) {
    return qname + "[" + std::to_string(pos) + "]";
  }
  return qname;
}

std::string srcml_reader::build_xpath(xmlNodePtr node) const {
  std::vector<std::string> segments;
  for (xmlNodePtr cur = node; cur; cur = cur->parent) {
    if (cur->type != XML_ELEMENT_NODE)
      continue;

    segments.push_back(build_segment(cur));
  }
  std::reverse(segments.begin(), segments.end());
  std::size_t total_size = 0;
  for (const auto &segment : segments) {
    total_size += 1 + segment.size();
  }
  std::string path;
  path.reserve(total_size);

  for (const auto &segment : segments) {
    path.push_back('/');
    path += segment;
  }
  return path;
}

std::string srcml_reader::node_qname(xmlNodePtr node) const {
  const char *local_name = reinterpret_cast<const char *>(node->name);
  if (!local_name)
    return "";

  if (node->ns && node->ns->prefix) {
    const char *prefix = reinterpret_cast<const char *>(node->ns->prefix);
    std::string out;
    out.reserve(std::strlen(prefix) + 1 + std::strlen(local_name));
    out += prefix;
    out.push_back(':');
    out += local_name;
    return out;
  }

  return std::string(local_name);
}

std::size_t srcml_reader::sibling_position(xmlNodePtr node) const {
  std::size_t index = 1;
  for (xmlNodePtr sib = node->prev; sib; sib = sib->prev) {
    if (sib->type != XML_ELEMENT_NODE)
      continue;
    if (sib->ns == node->ns && xmlStrEqual(sib->name, node->name)) {
      ++index;
    }
  }
  return index;
}

std::string srcml_reader::semantic_predicate(xmlNodePtr node) const {
  if (!node || node->type != XML_ELEMENT_NODE || !node->name)
    return "";

  const char *name = reinterpret_cast<const char *>(node->name);

  // src:unit[@filename="foo.cpp"]
  if (std::strcmp(name, "unit") == 0) {
    const xmlAttr *filename_attr = find_attribute(node, "filename");
    if (filename_attr &&
        filename_attr->children &&
        filename_attr->children->content) {
      std::string pred = "[@filename=";
      pred += escape_xpath_string(
          reinterpret_cast<const char *>(filename_attr->children->content));
      pred += "]";
      return pred;
    }
  }

  // src:function[src:name[text()="foo"]]
  if (std::strcmp(name, "function") == 0) {
    const xmlNode *name_node = find_child_element(node, "name");
    if (name_node && name_node->children && name_node->children->content) {
      std::string pred = "[src:name[text()=";
      pred += escape_xpath_string(
          reinterpret_cast<const char *>(name_node->children->content));
      pred += "]]";
      return pred;
    }
  }

  return "";
}

const xmlAttr * srcml_reader::find_attribute(xmlNodePtr node, const char *name) const {
  if (!node || !name)
    return nullptr;
  for (xmlAttrPtr attr = node->properties; attr; attr = attr->next) {
    if (xmlStrEqual(attr->name, BAD_CAST name)) {
      return attr;
    }
  }
  return nullptr;
}

const xmlNode * srcml_reader::find_child_element(xmlNodePtr node, const char *name) const {
  if (!node || !name)
    return nullptr;

  for (xmlNodePtr child = node->children; child; child = child->next) {
    if (child->type != XML_ELEMENT_NODE)
      continue;

    if (xmlStrEqual(child->name, BAD_CAST name)) {
      return child;
    }
  }

  return nullptr;
}

std::string srcml_reader::escape_xpath_string(const char *text) const {
  if (!text)
    return "\"\"";
  std::string out = "\"";
  for (const char *p = text; *p; ++p) {
    if (*p == '"') {
      out += "&quot;";
    } else {
      out.push_back(*p);
    }
  }
  out.push_back('"');
  return out;
}

// O(depth + siblings)
std::string srcml_reader::get_current_xpath() const {
  xmlNodePtr node = xmlTextReaderExpand(reader);
  if (!node)
    throw srcml_reader_error("Error expanding current node");

  // If reader somehow lands on a text node, use its parent element.
  while (node && node->type != XML_ELEMENT_NODE) {
    node = node->parent;
  }

  if (!node)
    throw srcml_reader_error("Current node is not an element");

  return build_xpath(node);
}
