cmake_minimum_required(VERSION 2.8)

project(snn_sim)


set(DIRS
    learning
    sim
    layers
    util
    config
    protos
    tuning_curves
    act_funcs
    serialize
	neurons
    synapses
    #filt
    #    conn
)

foreach(dir ${DIRS})
  add_subdirectory(${dir})
endforeach()

add_library(snn_sim SHARED ${SNN_SIM_SRCS})
include_directories(${CMAKE_BINARY_DIR}/protos)

SET_TARGET_PROPERTIES(snn_sim PROPERTIES COMPILE_FLAGS "-fPIC")

target_link_libraries(snn_sim
    protos
    ${PROTOBUF_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    m

)
install(TARGETS snn_sim  DESTINATION lib/ COMPONENT)
install(DIRECTORY ../snnlib DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
install(DIRECTORY ${CMAKE_BINARY_DIR}/snnlib/protos DESTINATION include/snnlib
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
