aux_source_directory(. DIR_SRCS)

add_executable(dmgPeek ${DIR_SRCS})

target_include_directories(dmgPeek PUBLIC ../dep/b64.c)

target_link_libraries(dmgPeek PUBLIC
	b64c
	zlib
)
