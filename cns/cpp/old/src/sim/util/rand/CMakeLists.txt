cmake_minimum_required(VERSION 2.8)

set(SOURCES
  common_rand_gen.h
  rand_funcs.h
)

# add directory name to sources
set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()

set(SIM_SRCS ${SIM_SRCS} ${DIR_SRCS} PARENT_SCOPE)
