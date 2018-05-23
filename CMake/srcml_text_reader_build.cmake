##
# @file srcml_text_reader_build.cmake
#
# Copyright (C) 2018 srcML, LLC. (www.srcML.org)
#
# This file is part of srcMLTextReader.
#
# srcMLTextReader is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# srcMLTextReader is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with srcMLTextReader.  If not, see <http://www.gnu.org/licenses/>.

get_filename_component(SRCML_TEXT_READER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
get_filename_component(SRCML_TEXT_READER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR} DIRECTORY)

# Compiler options
add_definitions("-std=c++14")

# find needed libraries
find_package(LibXml2 REQUIRED)

set(SRCML_TEXT_READER_INCLUDE_DIR
                       ${LIBXML2_INCLUDE_DIR}
                       ${SRCML_TEXT_READER_SOURCE_DIR}/src
    CACHE INTERNAL "Include directories for SRCML_TEXT_READER")

set(SRCML_TEXT_READER_LIBRARIES ${LIBXML2_LIBRARIES} srcml CACHE INTERNAL "Libraries for SRCML_TEXT_READER")

# include needed includes
include_directories(${SRCML_TEXT_READER_INCLUDE_DIR})

# Continue to build directory
add_subdirectory(${SRCML_TEXT_READER_SOURCE_DIR}/src ${SRCML_TEXT_READER_BINARY_DIR}/src)
