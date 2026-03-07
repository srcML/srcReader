//print_xpaths.cpp
#include "srcml_reader.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char **argv) {
  if (argc != 2 && argc != 3) {
    std::cerr << "usage: " << argv[0] << " <input.xml> [out.txt]\n";
    return 1;
  }

  const std::string in_filename = argv[1];
  const bool write_to_file = (argc == 3);
  const std::string out_filename = write_to_file ? argv[2] : "";

  try {
    srcml_reader reader(in_filename);

    std::ofstream out_file;
    std::ostream *out = &std::cout;

    if (write_to_file) {
      out_file.open(out_filename);
      if (!out_file) {
        throw std::runtime_error("failed to open output file: " + out_filename);
      }
      out = &out_file;
    }

    for (const srcml_node &node : reader) {
      // get_current_xpath() is only meaningful for real element positions.
      // So print one XPath per start element.
      if (!node.is_start()) {
        continue;
      }

      *out << reader.get_current_xpath() << '\n';
    }

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 2;
  }
}
