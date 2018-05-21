/*
  srcml_reader.hpp

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

#ifndef INCLUDED_SRCML_READER_HPP
#define INCLUDED_SRCML_READER_HPP

#include <srcml_node.hpp>

#include <libxml/xmlreader.h>

#include <string>
#include <memory>

class srcml_reader_error;

class srcml_reader {

private:

    void cleanup();

    xmlTextReaderPtr  reader;
    std::unique_ptr<srcml_node> current_node;

public:
    srcml_reader(const std::string & filename);
    ~srcml_reader();
    void read();

    const srcml_node & operator*();
    const srcml_node & operator++();
    srcml_node operator++(int);
};

#endif