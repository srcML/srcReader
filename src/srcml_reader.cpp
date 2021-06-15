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
  : reader(nullptr), offset(std::string::npos), current_node(), is_eof(false), iterator() {

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
  /// @todo handle empty elementa
  if(offset != std::string::npos && current_node->is_text()) {
    update_current_text_node();
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


