cmake_minimum_required(VERSION 3.10)

project(dmgPeek VERSION 1.0 DESCRIPTION "peek the dmg")

include(cmake/b64.cmake)

set(MYTAR_VERSION_MAJOR 1)
set(MYTAR_VERSION_MINOR 0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(dmgPeek mykolyblock.cc)
target_include_directories(dmgPeek PUBLIC ./dep/b64.c)
target_link_libraries(dmgPeek PUBLIC b64)

