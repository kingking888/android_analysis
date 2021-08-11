include_directories(../)
include_directories(../../source)
include_directories(../../external)


add_library(builtim_plugin STATIC
        global_offset_table_hook.cc
        global_offset_table_hook.h
        )