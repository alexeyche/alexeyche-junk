cmake_minimum_required(VERSION 2.8)

project(snn_sim)


set(DIRS
    #learn
    #sim
    #layers
    util
    config
    protos
    #prep
    #filt
    #    conn
)

foreach(dir ${DIRS})
  add_subdirectory(${dir})
endforeach()

add_library(snn_sim SHARED ${SNN_SIM_SRCS})
include_directories(${CMAKE_BINARY_DIR}/protos)

target_link_libraries(snn_sim 
    protos
    ${PROTOBUF_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT} 
    m

)
install(TARGETS snn_sim  DESTINATION lib/ COMPONENT)
install(DIRECTORY ../snnlib DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
