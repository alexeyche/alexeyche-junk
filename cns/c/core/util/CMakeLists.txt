cmake_minimum_required(VERSION 2.8)

# Define the files we need to compile.
# Anything not in this list will not be compiled into SNN_SIM.
set(SOURCES
    ini.c
    ini.h
    io.c
    io.h
    matrix.c
    matrix.h
    mem.c
    mem.h
    spikes_list.c
    spikes_list.h
    templates_clean.h
    templates.h
    util.c
    util_dlink_list.c
    util_dlink_list.h
    util.h
    util_vector.c
    util_vector.h

)

# add directory name to sources
set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()
# Append sources (with directory name) to list of all SNN_SIM sources (used at
# the parent scope).
set(SNN_SIM_SRCS ${SNN_SIM_SRCS} ${DIR_SRCS} PARENT_SCOPE)
