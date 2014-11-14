cmake_minimum_required(VERSION 2.8)

# Define the files we need to compile
# Anything not in this list will not be compiled into the output library
# Do not include test programs here
set(SOURCES
)

# add directory name to sources
set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()
# append sources (with directory name) to list of all SNN_SIM sources (used at the parent scope)
set(SNN_SIM_SRCS ${SNN_SIM_SRCS} ${DIR_SRCS} PARENT_SCOPE)

# executable
add_executable(convert_ts
    convert_ts.cpp
)


include_directories(${CMAKE_BINARY_DIR}/protos)
# link dependencies of executable
target_link_libraries(convert_ts
    protos
    ${PROTOBUF_LIBRARY}
    snn_sim
    m
)
install(TARGETS RUNTIME DESTINATION bin)
