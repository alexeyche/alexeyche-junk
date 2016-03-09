cmake_minimum_required(VERSION 2.8)

set(DIRS
    spdlog
    pbjson
)

foreach(dir ${DIRS})
  add_subdirectory(${dir})
endforeach()


set(SOURCES
    digamma.cpp
)

# add directory name to sources
set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()

set(DNN_SRCS ${DNN_SRCS} ${DIR_SRCS} PARENT_SCOPE)
