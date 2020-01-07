file(GLOB SRC_READER_SOURCE *.cpp)
file(GLOB SRC_READER_INCLUDE *.hpp)

# build_lib
#  Used to help with the creation of the srcML library.
#  - LIB_NAME is the name of the library and target
#  - LIB_TYPE is either STATIC or SHARED.
# 
macro(build_lib LIB_NAME LIB_TYPE)

    add_library(${LIB_NAME} ${LIB_TYPE} ${SRC_READER_SOURCE} ${SRC_READER_INCLUDE})

if("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xMSVC")
    set_target_properties(${LIB_NAME} PROPERTIES OUTPUT_NAME libsrcreader LINK_FLAGS_DEBUG "/SAFESEH:NO")
elseif(APPLE)
    set_target_properties(${LIB_NAME} PROPERTIES OUTPUT_NAME srcreader MACOSX_RPATH OFF)
else()
    set_target_properties(${LIB_NAME} PROPERTIES OUTPUT_NAME srcreader)
endif()

endmacro()

build_lib(srcreader_static STATIC)
build_lib(srcreader_shared SHARED)
target_link_libraries(srcreader_shared PRIVATE ${SRC_READER_LIBRARIES})

install(TARGETS srcreader_shared srcreader_static RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES ${SRC_READER_INCLUDE} DESTINATION include/srcreader)
