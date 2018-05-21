/*
  srcml_writer.cpp

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

#include <srcml_writer.hpp>

#include <libxml/xmlwriter.h>

class srcml_writer_error : public std::runtime_error {
public:
  srcml_writer_error(const std::string & what_arg) : std::runtime_error(what_arg) {}

};

void srcml_writer::cleanup() {
  if(archive) {
    srcml_archive_close(archive);
    srcml_archive_free(archive);
    archive = nullptr;
  }

  if(unit) {
    srcml_unit_free(unit);
    unit = nullptr;
  }
}

template<class... message_type>
void srcml_writer::check_srcml_error(int error_code, bool perform_cleanup, const message_type &... message) {
  if(error_code == SRCML_STATUS_OK) return;
  
  if(perform_cleanup) cleanup();

  std::string error_message;
  for(const std::string & msg : { message... }) {
    error_message += msg;
  }

  throw srcml_writer_error(error_message);
}


srcml_writer::srcml_writer(const std::string & filename)
  : archive(nullptr), unit(nullptr), saved_characters() {

    archive = srcml_archive_create();
    if(!archive) throw srcml_writer_error("Failure creating srcML Archive");
    check_srcml_error(srcml_archive_write_open_filename(archive, filename.c_str(), 0), true, "Unable to open: ", filename.c_str());

}

srcml_writer::~srcml_writer() {
  cleanup();
}

bool srcml_writer::write(const srcml_node & node) {
  return write_process_map[node.type](node);
}

// typename std::unordered_map<std::string, std::function<int (srcml_archive *, const char *)::const_iterator archive_attr_map_citr;
// std::unordered_map<std::string, std::function<int (srcml_archive *, const char *)>> archive_attr_map = {

//     { "url", srcml_archive_set_url },
// };

void srcml_writer::set_unit_attr(srcml_unit * unit, const std::list<srcml_node::srcml_attr> & properties) {

  typedef std::unordered_map<std::string, std::function<int (srcml_unit *, const char *)>>::const_iterator unit_attr_map_citr;
  static std::unordered_map<std::string, std::function<int (srcml_unit *, const char *)>> unit_attr_map = {
    { "language", srcml_unit_set_language },
    { "filename", srcml_unit_set_filename },
  };

  for(const srcml_node::srcml_attr & attr : properties) {

    unit_attr_map_citr citr = unit_attr_map.find(attr.name);
    if(citr != unit_attr_map.end()) {
      check_srcml_error(unit_attr_map[attr.name](unit, attr.value ? attr.value->c_str() : 0), false, "Error setting archive", attr.name.c_str());
    } else {
      throw srcml_writer_error("Unimplemented attribute: " + attr.name);
    }

  }

}

bool srcml_writer::setup_archive(const srcml_node & node) {
  const std::string CPP_NAMEPACE = "http://www.srcML.org/srcML/cpp";

    unit = srcml_unit_create(archive);
    if(!unit) throw srcml_writer_error("Failure creating srcML Unit");

  for(const srcml_node::srcml_ns & ns : node.ns_def) {
    if(ns.href == CPP_NAMEPACE) continue;
    check_srcml_error(srcml_archive_register_namespace(archive, node.ns.prefix ? node.ns.prefix->c_str() : 0, ns.href.c_str()),
                      false, "Error error registering namespace: ", ns.href.c_str());
  }


  set_unit_attr(unit, node.properties);

  write_process_map[srcml_node::srcml_node_type::START] = std::bind(&srcml_writer::write_start_first, this, std::placeholders::_1);
  return true;
}

bool srcml_writer::write_start_first(const srcml_node & node) {

  write_process_map[srcml_node::srcml_node_type::START] = std::bind(&srcml_writer::write_start, this, std::placeholders::_1);
  write_process_map[srcml_node::srcml_node_type::TEXT] = std::bind(&srcml_writer::write_text, this, std::placeholders::_1);

  if(node.name != "unit") {
    write_process_map[srcml_node::srcml_node_type::TEXT](srcml_node(saved_characters));
    write_process_map[srcml_node::srcml_node_type::START](node);
    return true;
  }

  check_srcml_error(srcml_archive_enable_full_archive(archive), false, "Error enabling archive");

  set_unit_attr(unit, node.properties);

  return true;

}

bool srcml_writer::write_start(const srcml_node & node) {

  if(node.name != "unit") {
    srcml_write_start_element(unit, node.ns.prefix ? node.ns.prefix->c_str() : 0, node.name.c_str(), 0);

    for(const srcml_node::srcml_ns ns : node.ns_def) {
      check_srcml_error(srcml_write_namespace(unit, ns.prefix ? ns.prefix->c_str() : 0, ns.href.c_str()), "Error writing namespace", ns.href.c_str());
    }

    for(const srcml_node::srcml_attr attr : node.properties) {
      check_srcml_error(srcml_write_attribute(unit, 0, attr.name.c_str(), 0, attr.value ? attr.value->c_str() : 0), "Error writing attribute", attr.name.c_str());
    }

  } else {
    set_unit_attr(unit, node.properties);
  }

  if(node.is_empty) write_process_map[srcml_node::srcml_node_type::END](node);
  return true;
}

bool srcml_writer::write_end(const srcml_node & node) {

  if(node.name != "unit") {
    srcml_write_end_element(unit);
    return true;
  }

  check_srcml_error(srcml_archive_write_unit(archive, unit), false, "Error writing unit");
  srcml_unit_free(unit);
  unit = srcml_unit_create(archive);
  if(!unit) throw srcml_writer_error("Error creating unit");

  return true;
}

bool srcml_writer::write_text_first(const srcml_node & node) {

  if(!node.content) return true;

  saved_characters += *node.content;
  return true;

}

bool srcml_writer::write_text(const srcml_node & node) {
  srcml_write_string(unit, node.content ? node.content->c_str() : 0);
  return true;
}

bool srcml_writer::write_error(const srcml_node & node) {
  throw srcml_writer_error("Invalid srcml_node type: " + std::to_string(node.type));
  return false;
}

