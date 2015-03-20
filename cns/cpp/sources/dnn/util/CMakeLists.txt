cmake_minimum_required(VERSION 2.8)

# Define the files we need to compile.
# Anything not in this list will not be compiled into SNN_SIM.
set(DIRS
)

foreach(dir ${DIRS})
  add_subdirectory(${dir})
endforeach()


set(SOURCES
    matrix.cpp
    matrix.h
    util.h
    util.cpp
    json.cpp
    #spikes_list.cpp
    #spikes_queue.cpp
)

# add directory name to sources
set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()
# Append sources (with directory name) to list of all SNN_SIM sources (used at
# the parent scope).
set(DNN_SRCS ${DNN_SRCS} ${DIR_SRCS} PARENT_SCOPE)
