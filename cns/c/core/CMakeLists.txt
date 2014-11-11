cmake_minimum_required(VERSION 2.8)

project(snn_sim)


set(DIRS
    learn
    sim
    layers
    util
    #    conn
)

foreach(dir ${DIRS})
  add_subdirectory(${dir})
endforeach()


add_library(snn_sim SHARED ${SNN_SIM_SRCS} "constants.c")

target_link_libraries(snn_sim ${CMAKE_THREAD_LIBS_INIT} m)
install(TARGETS snn_sim  DESTINATION lib/ COMPONENT)
install(FILES ${SNN_SIM_SRCS} DESTINATION include/snn_sim)
