cmake_minimum_required(VERSION 2.8)

set(SOURCES
    sim.cpp
    layer.cpp
    network.cpp
    global_ctx.cpp
    reward_control.cpp
)

set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()

set(DNN_SRCS ${DNN_SRCS} ${DIR_SRCS} PARENT_SCOPE)
