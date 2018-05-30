/*
  srcml_node.hpp

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

#ifndef INCLUDED_SRCML_NODE_HPP
#define INCLUDED_SRCML_NODE_HPP

#include <srcml.h>

#include <string>
#include <list>
#include <map>
#include <unordered_map>
#include <memory>

#include <boost/optional.hpp>

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

class srcml_node {

public:

  class srcml_namespace {

  public:

    std::string uri;
    boost::optional<std::string> prefix;

    srcml_namespace(const std::string & uri = std::string(),
                    const boost::optional<std::string> & prefix = boost::optional<std::string>());
    srcml_namespace(xmlNsPtr ns);

  };

  static std::shared_ptr<srcml_namespace> SRC_NAMESPACE;
  static std::shared_ptr<srcml_namespace> CPP_NAMESPACE;
  static std::unordered_map<std::string, std::shared_ptr<srcml_namespace>> namespaces;

  class srcml_attribute {

  public:

    std::string name;
    boost::optional<std::string> value;
    std::shared_ptr<srcml_namespace> ns;

    srcml_attribute(xmlAttrPtr attribute);
    srcml_attribute(const std::string & name = std::string(),
                    std::shared_ptr<srcml_namespace> ns = SRC_NAMESPACE,
                    boost::optional<std::string> value = boost::optional<std::string>());

    std::string full_name() const;

    bool operator==(const srcml_attribute & that) const;
    bool operator!=(const srcml_attribute & that) const;

  };

  enum srcml_node_type : unsigned int  { OTHER = 0, START = 1, END = 2, TEXT = 3 };

  typedef std::map<std::string, srcml_attribute> srcml_attribute_map;
  typedef std::pair<std::string, srcml_attribute> srcml_attribute_map_pair;
  typedef std::map<std::string, srcml_attribute>::const_iterator srcml_attribute_map_citr;
  typedef std::map<std::string, srcml_attribute>::iterator srcml_attribute_map_itr;

  srcml_node_type type;
  std::string name;
  std::shared_ptr<srcml_namespace> ns;
  boost::optional<std::string> content;
  std::list<std::shared_ptr<srcml_namespace>> ns_definition;
  srcml_attribute_map attributes;
  bool is_empty;

  unsigned short extra;

  static std::shared_ptr<srcml_namespace> get_namespace(xmlNsPtr ns);

public:

  srcml_node();
  srcml_node(const xmlNode & node, xmlElementType xml_type);
  srcml_node(const std::string & text);

  ~srcml_node();

  std::string full_name() const;

  bool operator==(const srcml_node & node) const;
  bool operator!=(const srcml_node & node) const;

  bool is_text() const;

};

#endif
