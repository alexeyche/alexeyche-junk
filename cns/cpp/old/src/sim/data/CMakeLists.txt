cmake_minimum_required(VERSION 2.8)

# Define the files that we need to compile.
# Anything not in this list will not be compiled into MLPACK.
set(SOURCES
  load.h
  load_impl.h
  save.h
  save_impl.h
)

# add directory name to sources
set(DIR_SRCS)
foreach(file ${SOURCES})
    set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()

set(SIM_SRCS ${SIM_SRCS} ${DIR_SRCS} PARENT_SCOPE)
