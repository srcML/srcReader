#include "srcml_node.hpp"
#include "srcml_reader.hpp"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <srcdiff.xml>\n";
    return 1;
  }

  std::string filename = argv[1];
  std::cout << filename << "\n";

  try {
    srcml_reader reader(filename);
    for (const srcml_node &node : reader) {
      auto dbg = node; // <-- copy for debugger
      std::cout << node << "\n---------------------------------------------\n";
      for (const srcml_node &node : reader) {
        auto dbg_type = node.type;
        auto dbg_name = node.name;
        auto dbg_empty = node.empty;
        auto dbg_extra = node.extra;
        auto dbg_content = node.content ? *node.content : std::string("<none>");
        auto x = 0;
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 2;
  }

  return 0;
}
