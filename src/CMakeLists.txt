file(GLOB READER_SOURCE *.cpp)
file(GLOB READER_INCLUDE *.hpp)

# build_lib
#  Used to help with the creation of the srcML library.
#  - LIB_NAME is the name of the library and target
#  - LIB_TYPE is either STATIC or SHARED.
# 
macro(build_lib LIB_NAME LIB_TYPE)

    add_library(${LIB_NAME} ${LIB_TYPE} ${READER_SOURCE} ${READER_INCLUDE})

if("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xMSVC")
    set_target_properties(${LIB_NAME} PROPERTIES OUTPUT_NAME libsrcml_text_reader LINK_FLAGS_DEBUG "/SAFESEH:NO")
elseif(APPLE)
    set_target_properties(${LIB_NAME} PROPERTIES OUTPUT_NAME srcml_text_reader MACOSX_RPATH OFF)
else()
    set_target_properties(${LIB_NAME} PROPERTIES OUTPUT_NAME srcml_text_reader)
endif()

endmacro()

build_lib(srcml_text_reader_static STATIC)
build_lib(srcml_text_reader_shared SHARED)
target_link_libraries(srcml_text_reader_shared PRIVATE ${SRCML_TEXT_READER_LIBRARIES})

install(TARGETS srcml_text_reader_shared srcml_text_reader_static RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES ${HANDLER_INCLUDE} DESTINATION include/srcml_text_reader)
