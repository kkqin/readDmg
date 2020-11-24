aux_source_directory(. DIR_SRCS)

add_executable(dmgRead ${DIR_SRCS})

target_include_directories(dmgRead PUBLIC ../dep/b64.c)

target_link_libraries(dmgRead PUBLIC
	b64c
)
