cmake_minimum_required(VERSION 3.10)

project(dmgPeek VERSION 1.0 DESCRIPTION "peek the dmg")

set(MYTAR_VERSION_MAJOR 1)
set(MYTAR_VERSION_MINOR 0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(cmake/b64.cmake)

add_subdirectory(src)
#add_executable(dmgPeek mykolyblock.cc)
#target_link_libraries(dmgPeek PUBLIC b64)
#target_include_directories(dmgPeek PUBLIC ./dep/b64.c)

