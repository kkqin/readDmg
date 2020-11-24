set(B64_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dep/b64.c)
include_directories("${B64_DIR}")
add_subdirectory(${B64_DIR} dep/b64.c)
