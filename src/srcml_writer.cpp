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
  : archive(nullptr) {

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


bool srcml_writer::setup_archive(const srcml_node & node) {

  for(const srcml_node::srcml_ns & ns : node.ns_def) {
    check_srcml_error(srcml_archive_register_namespace(archive, node.ns.prefix ? node.ns.prefix->c_str() : 0, ns.href.c_str()),
                     false, "Error error registering namespace: ", ns.href.c_str());
  }

  static std::unordered_map<std::string, std::function<int (srcml_archive *, const char *)>> attr_map = {
    { "language", srcml_archive_set_language },
    { "url", srcml_archive_set_url },
  } ;

  for(const srcml_node::srcml_attr & attr : node.properties) {

    try {
      check_srcml_error(attr_map[attr.name](archive, attr.value ? attr.value->c_str() : 0), false, "Error setting archive", attr.name.c_str());
    } catch(const std::out_of_range & error) {
      throw srcml_writer_error("Unimplemented archive attribute: " + attr.name);      
    }

  }

  write_process_map[srcml_node::srcml_node_type::START] = std::bind(&srcml_writer::write_start, this, std::placeholders::_1);

}
bool srcml_writer::write_start(const srcml_node & node) {}
bool srcml_writer::write_end(const srcml_node & node) {}
bool srcml_writer::write_text(const srcml_node & node) {}
bool srcml_writer::write_error(const srcml_node & node) {
  throw srcml_writer_error("Invalid srcml_node type: " + std::to_string(node.type));
  return false;
}

//   switch (node.type) {
//     case XML_READER_TYPE_ELEMENT: {

//         if(node.name == "unit") {

//           if(configure_archive) {

//             // might want to switch lambda instead
//             configure_archive = false;

//           } else {

//           }


//         } else {

//         // start the element
//         srcml_write_start_element(wstate->unit, node.ns.prefix ? node.ns.prefix->c_str() : 0, node.name.c_str(), 0);

//         // copy all the attributes
//         const std::list<srcml_node::srcml_attr> & attributes = node.properties;
//         for(const srcml_node::srcml_attr attr : attributes) {

//             srcml_write_attribute(wstate->unit, 0, attr.name.c_str(), 0, attr.value ? attr.value->c_str() : 0);

//         // end now if this is an empty element
//         if (node.is_empty)
//             srcml_write_end_element(wstate->unit);

//         break;
//       }


//     }

//     case XML_READER_TYPE_END_ELEMENT: {
//         srcml_write_end_element(wstate->unit);
//         break;
//     }

//     case XML_READER_TYPE_COMMENT: {
//         break;
//     }

//     case XML_READER_TYPE_TEXT:
//     case XML_READER_TYPE_SIGNIFICANT_WHITESPACE: {

//         srcml_write_string(wstate->unit, node.content ? node.content->c_str() : 0);
//         break;
//     }

//     default: {
//         break;
//     }
//   }
// }
