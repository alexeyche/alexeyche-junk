cmake_minimum_required(VERSION 2.8)

set(DIRS
    dumptool
)

foreach(dir ${DIRS})
    add_subdirectory(${dir})
endforeach()

set(DNN_SRCS ${DNN_SRCS} ${DIR_SRCS} PARENT_SCOPE)
