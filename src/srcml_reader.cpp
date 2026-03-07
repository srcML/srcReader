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
#include <srcml_reader.hpp>

#include <algorithm>
#include <cstring>
#include <vector>
#include <string>

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

  } else if (issue_end_tag) {
    issue_end_tag = false;

    current_node = std::make_unique<srcml_node>(*current_node);
    current_node->type = srcml_node::srcml_node_type::END;
    current_node->attributes.clear();
    current_node->ns_definition.clear();

    if (!element_stack.empty())
      element_stack.pop();

    if (!xpath_stack.empty())
      xpath_stack.pop_back();

    if (!child_count_stack.empty())
      child_count_stack.pop_back();

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

    if (current_node->is_start()) {
      element_stack.push(current_node->full_name());

      xmlNodePtr expanded = xmlTextReaderExpand(reader);
      if (!expanded)
        throw srcml_reader_error("Error expanding current node");

      std::size_t seen_sibling_index = 1;

      // Count this element among its parent's direct child elements
      // using only its qualified element name, not its semantic predicate.
      if (!child_count_stack.empty()) {
        const std::string sibling_key = node_qname(expanded);
        seen_sibling_index = ++child_count_stack.back()[sibling_key];
      }

      xpath_stack.push_back(build_segment(expanded, seen_sibling_index));

      // Prepare a fresh child-count frame for this element's own children.
      child_count_stack.emplace_back();
    } else if (current_node->is_end()) {
      if (!element_stack.empty())
        element_stack.pop();

      if (!xpath_stack.empty())
        xpath_stack.pop_back();

      if (!child_count_stack.empty())
        child_count_stack.pop_back();
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

bool srcml_reader::has_same_named_sibling(xmlNodePtr node) const {
  if (!node)
    return false;

  for (xmlNodePtr sib = node->prev; sib; sib = sib->prev) {
    if (sib->type != XML_ELEMENT_NODE)
      continue;
    if (same_expanded_name(sib, node))
      return true;
  }

  for (xmlNodePtr sib = node->next; sib; sib = sib->next) {
    if (sib->type != XML_ELEMENT_NODE)
      continue;
    if (same_expanded_name(sib, node))
      return true;
  }

  return false;
}

std::string srcml_reader::get_current_inner_text() const {
  xmlNodePtr subtree = xmlTextReaderExpand(reader);
  if (!subtree)
    throw srcml_reader_error("Error expanding current node");

  std::string result;
  collect_text(subtree->children, result); // children only = "inside the tag"
  return result;
}

std::string srcml_reader::build_segment(xmlNodePtr node,
                                        std::size_t seen_sibling_index) const {
  const std::string qname = node_qname(node);
  const std::string predicate = semantic_predicate(node);

  if (!predicate.empty()) {
    return qname + predicate;
  }

  // If streaming state already knows this is the 2nd/3rd/etc sibling
  // with the same expanded name under the same parent, trust that.
  if (seen_sibling_index > 1) {
    return qname + "[" + std::to_string(seen_sibling_index) + "]";
  }

  // For the first sibling, we still want [1] when there are later
  // same-named siblings. Keep the DOM-based fallback for that case.
  if (has_same_named_sibling(node)) {
    return qname + "[" + std::to_string(sibling_position(node)) + "]";
  }

  return qname;
}


const char *srcml_reader::canonical_prefix(xmlNodePtr node) const {
  if (!node || !node->ns || !node->ns->href)
    return nullptr;

  const char *uri = reinterpret_cast<const char *>(node->ns->href);

  if (std::strcmp(uri, "http://www.srcML.org/srcML/src") == 0)
    return "src";
  if (std::strcmp(uri, "http://www.srcML.org/srcDiff") == 0)
    return "diff";
  if (std::strcmp(uri, "http://www.srcML.org/srcMove") == 0)
    return "mv";

  // Fallback for any other namespace actually carrying a prefix.
  if (node->ns->prefix)
    return reinterpret_cast<const char *>(node->ns->prefix);

  return nullptr;
}

std::string srcml_reader::node_qname(xmlNodePtr node) const {
  if (!node || !node->name)
    return "";

  const char *local_name = reinterpret_cast<const char *>(node->name);
  const char *prefix = canonical_prefix(node);

  if (!prefix)
    return std::string(local_name);

  std::string out;
  out.reserve(std::strlen(prefix) + 1 + std::strlen(local_name));
  out += prefix;
  out.push_back(':');
  out += local_name;
  return out;
}

std::size_t srcml_reader::sibling_position(xmlNodePtr node) const {
  std::size_t index = 1;
  for (xmlNodePtr sib = node->prev; sib; sib = sib->prev) {
    if (sib->type != XML_ELEMENT_NODE)
      continue;
    if (same_expanded_name(sib, node))
      ++index;
  }
  return index;
}

std::string srcml_reader::get_node_inner_text(xmlNodePtr node) const {
  if (!node)
    return "";
  std::string result;
  collect_text(node->children, result);
  return result;
}

bool srcml_reader::same_expanded_name(xmlNodePtr a, xmlNodePtr b) const {
  if (!a || !b)
    return false;
  if (!xmlStrEqual(a->name, b->name))
    return false;

  const xmlChar *a_href = (a->ns && a->ns->href) ? a->ns->href : nullptr;
  const xmlChar *b_href = (b->ns && b->ns->href) ? b->ns->href : nullptr;

  if (!a_href && !b_href)
    return true;
  if (!a_href || !b_href)
    return false;

  return xmlStrEqual(a_href, b_href);
}

bool srcml_reader::is_simple_name_for_xpath(const std::string &s) const {
  if (s.empty())
    return false;
  // Conservative rule: only allow plain identifiers and a few safe symbols.
  for (char ch : s) {
    if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == ':' || ch == '~')
      continue;
    return false;
  }
  return true;
}

bool srcml_reader::is_structurally_simple_name(xmlNodePtr name_node) const {
  if (!name_node)
    return false;
  for (xmlNodePtr child = name_node->children; child; child = child->next) {
    if (child->type == XML_TEXT_NODE || child->type == XML_CDATA_SECTION_NODE)
      continue;
    // Any element child means mixed structure: reject.
    return false;
  }

  return true;
}

std::string srcml_reader::semantic_predicate(xmlNodePtr node) const {
  if (!node || node->type != XML_ELEMENT_NODE || !node->name)
    return "";

  const char *local_name = reinterpret_cast<const char *>(node->name);
  static const char *SRC_URI = "http://www.srcML.org/srcML/src";

  // Nested file unit inside an archive:
  //   /src:unit/src:unit[@filename="main.cpp"]
  // Keep the root archive unit as just /src:unit.
  if (std::strcmp(local_name, "unit") == 0) {
    const bool parent_is_unit =
        node->parent &&
        node->parent->type == XML_ELEMENT_NODE &&
        node->parent->name &&
        xmlStrEqual(node->parent->name, BAD_CAST "unit");

    if (!parent_is_unit)
      return "";

    const xmlAttr *filename_attr = find_attribute(node, "filename");
    if (!filename_attr || !filename_attr->children ||
        !filename_attr->children->content) {
      return "";
    }

    std::string pred = "[@filename=";
    pred += escape_xpath_string(
        reinterpret_cast<const char *>(filename_attr->children->content));
    pred += "]";
    return pred;
  }

  auto make_name_predicate = [&](const xmlNode *name_node) -> std::string {
    if (!name_node)
      return "";

    xmlNodePtr mutable_name_node = const_cast<xmlNodePtr>(name_node);
    if (!is_structurally_simple_name(mutable_name_node))
      return "";

    const std::string name_text = get_node_inner_text(mutable_name_node);
    if (name_text.empty() || !is_simple_name_for_xpath(name_text))
      return "";

    std::string pred = "[src:name=";
    pred += escape_xpath_string(name_text.c_str());
    pred += "]";
    return pred;
  };

  // Named callable/program elements.
  if (std::strcmp(local_name, "function") == 0 ||
      std::strcmp(local_name, "function_decl") == 0 ||
      std::strcmp(local_name, "constructor") == 0 ||
      std::strcmp(local_name, "destructor") == 0) {
    return make_name_predicate(find_child_element(node, "name", SRC_URI));
  }

  // Named declaration-like elements.
  if (std::strcmp(local_name, "decl") == 0 ||
      std::strcmp(local_name, "param") == 0 ||
      std::strcmp(local_name, "typedef") == 0) {
    return make_name_predicate(find_child_element(node, "name", SRC_URI));
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

const xmlNode *srcml_reader::find_child_element(xmlNodePtr node,
                                                const char *local_name,
                                                const char *namespace_uri) const {
  if (!node || !local_name)
    return nullptr;

  for (xmlNodePtr child = node->children; child; child = child->next) {
    if (child->type != XML_ELEMENT_NODE)
      continue;

    if (!xmlStrEqual(child->name, BAD_CAST local_name))
      continue;

    if (!namespace_uri)
      return child;

    if (child->ns && child->ns->href &&
        std::strcmp(reinterpret_cast<const char *>(child->ns->href),
                    namespace_uri) == 0) {
      return child;
    }
  }
  return nullptr;
}

std::string srcml_reader::escape_xpath_string(const char *text) const {
  if (!text)
    return "\"\"";

  const std::string s(text);

  // Prefer double quotes if possible.
  if (s.find('"') == std::string::npos) {
    return "\"" + s + "\"";
  }

  // Otherwise prefer single quotes.
  if (s.find('\'') == std::string::npos) {
    return "'" + s + "'";
  }

  // If both quote types appear, build concat(...)
  std::string out = "concat(";
  bool first = true;
  std::size_t start = 0;

  for (std::size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '"' || s[i] == '\'') {
      if (i > start) {
        if (!first) out += ", ";
        out += "\"" + s.substr(start, i - start) + "\"";
        first = false;
      }

      if (!first) out += ", ";
      if (s[i] == '"')
        out += "'\"'";
      else
        out += "\"'\"";
      first = false;

      start = i + 1;
    }
  }

  if (start < s.size()) {
    if (!first) out += ", ";
    out += "\"" + s.substr(start) + "\"";
  }

  out += ")";
  return out;
}

std::string srcml_reader::get_current_xpath() const {
  if (xpath_stack.empty())
    return "";
  std::size_t total_size = 0;
  for (const auto &segment : xpath_stack) {
    total_size += 1 + segment.size();
  }
  std::string path;
  path.reserve(total_size);
  for (const auto &segment : xpath_stack) {
    path.push_back('/');
    path += segment;
  }
  return path;
}
