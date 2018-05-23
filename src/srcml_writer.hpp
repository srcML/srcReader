/*
  srcml_writer.hpp

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

#ifndef INCLUDED_SRCML_WRITER_HPP
#define INCLUDED_SRCML_WRITER_HPP

#include <srcml_node.hpp>

#include <srcml.h>

#include <string>
#include <stdexcept>
#include <unordered_map>
#include <functional>

class srcml_writer_error;

class srcml_writer {

private:
    bool setup_archive(const srcml_node & node);
    bool write_start_first(const srcml_node & node);
    bool write_start(const srcml_node & node);
    bool write_end(const srcml_node & node);
    bool write_text_first(const srcml_node & node);
    bool write_text(const srcml_node & node);
    bool write_error(const srcml_node & node);

    std::unordered_map<int, std::function<bool (const srcml_node & node)>> write_process_map = { 
        { srcml_node::srcml_node_type::START, std::bind(&srcml_writer::setup_archive, this, std::placeholders::_1) },
        { srcml_node::srcml_node_type::END, std::bind(&srcml_writer::write_end, this, std::placeholders::_1) },
        { srcml_node::srcml_node_type::TEXT, std::bind(&srcml_writer::write_text_first, this, std::placeholders::_1) },
        { srcml_node::srcml_node_type::OTHER, std::bind(&srcml_writer::write_error, this, std::placeholders::_1) },
    };

    void cleanup();

    template<class... message_type>
    void check_srcml_error(int error_code, bool perform_cleanup, const message_type &... message);
    void set_unit_attr(srcml_unit * unit, const srcml_node::srcml_attribute_map & attributes);

    srcml_archive * archive;
    srcml_unit * unit;
    std::string saved_characters;

public:
    srcml_writer(const std::string & filename);
    ~srcml_writer();
    bool write(const srcml_node & node);

};

#endif