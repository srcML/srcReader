/*
  srcml_reader.hpp

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

#ifndef INCLUDED_SRCML_READER_HPP
#define INCLUDED_SRCML_READER_HPP

#include <srcml_node.hpp>

#include <libxml/xmlreader.h>

#include <string>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

class srcml_reader_error;

class srcml_reader {
public:
      class srcml_reader_iterator {
      private:
        srcml_reader * reader;
        srcml_reader_iterator(srcml_reader * reader = nullptr);
      public:
        const srcml_node & operator*() const;
        srcml_node & operator*();
        const srcml_node * operator->() const;
        srcml_node * operator->();
        const srcml_node & operator++();
        srcml_node operator++(int);
        bool operator!=(const srcml_reader_iterator & that) const;

        friend class srcml_reader;
  };
private:

  void cleanup();
  bool read();
  void update_current_text_node();

  xmlTextReaderPtr reader;
  std::string::size_type offset;
  std::unique_ptr<srcml_node> saved_node;

  bool issue_end_tag;

  std::unique_ptr<srcml_node> current_node;
  bool is_eof;

  srcml_reader_iterator iterator;

  std::stack<std::string> element_stack;
  std::vector<std::string> xpath_stack;
  std::vector<std::unordered_map<std::string, std::size_t>> child_count_stack;

  // helpers for xpath generation
  bool is_structurally_simple_name(xmlNodePtr name_node) const;
  bool is_simple_name_for_xpath(const std::string &s) const;
  std::string build_segment(xmlNodePtr node,
                            const std::string &predicate,
                            std::size_t seen_sibling_index) const;
  std::string node_qname(xmlNodePtr node) const;
  std::size_t sibling_position(xmlNodePtr node) const;
  std::string semantic_predicate(xmlNodePtr node) const;
  const xmlAttr * find_attribute(xmlNodePtr node, const char *name) const;
  std::string escape_xpath_string(const char *text) const;
  const char * canonical_prefix(xmlNodePtr node) const;
  const xmlNode * find_child_element(xmlNodePtr node, const char *local_name,
                                    const char *namespace_uri = nullptr) const;
  bool has_same_named_sibling(xmlNodePtr node) const;
  std::string get_node_inner_text(xmlNodePtr node) const;
  bool same_expanded_name(xmlNodePtr a, xmlNodePtr b) const;
  bool root_kind_known;
  bool root_is_archive;
  bool has_attribute(xmlNodePtr node, const char *name) const;
  bool is_root_archive_unit(xmlNodePtr node) const;

public:
  srcml_reader(const std::string & filename);
  ~srcml_reader();

  const std::stack<std::string> & get_element_stack() const;

  srcml_reader_iterator begin();
  srcml_reader_iterator end();
  xmlDocPtr get_current_doc() const;
  xmlNodePtr expand_current_node() const;
  std::string get_current_xpath() const;

  operator bool() const;
  std::string get_current_inner_text() const;
};

#endif
