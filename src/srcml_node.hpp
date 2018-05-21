/*
  srcml_node.hpp

  Copyright (C) 2018 srcML, LLC. (www.srcML.org)

  This file is part of a translator from source code to srcIdentity

  srcIdentity is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  srcIdentity is distributed in the hope that it will be useful,
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
#include <memory>

#include <boost/optional.hpp>

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

class srcml_node {

public:

  class srcml_ns {

  public:

    std::string href;
    boost::optional<std::string> prefix;

    srcml_ns(const std::string & href = std::string(), const boost::optional<std::string> & prefix = boost::optional<std::string>())
      : href(href), prefix(prefix) {}

    srcml_ns(const srcml_ns & ns);

  };

  class srcml_attr {

  public:

    std::string name;
    boost::optional<std::string> value;

    srcml_attr(const std::string & name = std::string(), const boost::optional<std::string> & value = boost::optional<std::string>())
      : name(name), value(value) {}

    bool operator==(const srcml_attr & attr) const;
    bool operator!=(const srcml_attr & attr) const;

  };

  enum srcml_node_type : unsigned int  { OTHER = 0, START = 1, END = 2, TEXT = 3 };

  srcml_node_type type;
  std::string name;
  srcml_ns ns;
  boost::optional<std::string> content;
  std::list<srcml_ns> ns_def;
  std::list<srcml_attr> properties;
  bool is_empty;

  unsigned short extra;


public:

  srcml_node(const xmlNode & node);
  srcml_node(const srcml_node & node);

  ~srcml_node();

  bool operator==(const srcml_node & node) const;

  bool is_text() const;

};

#endif
