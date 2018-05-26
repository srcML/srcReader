##
# @file srcml_text_reader_build.cmake
#
# Copyright (C) 2018 srcML, LLC. (www.srcML.org)
#
# This file is part of srcReader.
#
# srcReader is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# srcReader is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with srcReader.  If not, see <http://www.gnu.org/licenses/>.

get_filename_component(SRC_READER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
get_filename_component(SRC_READER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR} DIRECTORY)

# Compiler options
add_definitions("-std=c++14")

# find needed libraries
find_package(LibXml2 REQUIRED)

set(SRC_READER_INCLUDE_DIR
                       ${LIBXML2_INCLUDE_DIR}
                       ${SRC_READER_SOURCE_DIR}/src
    CACHE INTERNAL "Include directories for SRC_READER")

set(SRC_READER_LIBRARIES ${LIBXML2_LIBRARIES} srcml CACHE INTERNAL "Libraries for SRC_READER")

# include needed includes
include_directories(${SRC_READER_INCLUDE_DIR})

# Continue to build directory
add_subdirectory(${SRC_READER_SOURCE_DIR}/src ${SRC_READER_BINARY_DIR}/src)
