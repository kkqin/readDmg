cmake_minimum_required(VERSION 3.10)

project(dmgPeek VERSION 1.0 DESCRIPTION "peek the dmg")

set(MYTAR_VERSION_MAJOR 1)
set(MYTAR_VERSION_MINOR 0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(src)

include(CMakeDependentOption)
cmake_dependent_option(BUILD_TESTS 
	"Build tests" ON 
	"BUILD_TESTING;CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" ON)

if(BUILD_TESTS)
        set(test-pattern "test/*.cc" "src/core_st.cc")
        file(GLOB test-sources ${test-pattern})
        add_executable(dmg-read-test ${test-sources})
        target_include_directories(dmg-read-test PUBLIC ./src/)
        target_link_libraries(dmg-read-test z)
endif(BUILD_TESTS)

