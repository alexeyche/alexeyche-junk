cmake_minimum_required(VERSION 2.8)

project(dnn)


# Define the files we need to compile.
# Anything not in this list will not be compiled into DNN.
set(SOURCES

)

# add directory name to sources
set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()
# Append sources (with directory name) to list of all DNN sources (used at
# the parent scope).
set(DNN_SRCS ${DIR_SRCS})

set(DIRS
    io
    protos
    util
    base
    contrib
    neurons
    inputs
)

foreach(dir ${DIRS})
  add_subdirectory(${dir})
endforeach()

add_library(dnn SHARED ${DNN_SRCS})
include_directories(${CMAKE_BINARY_DIR}/protos)

SET_TARGET_PROPERTIES(dnn PROPERTIES COMPILE_FLAGS "-fPIC")

target_link_libraries(dnn
    protos
    ${PROTOBUF_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    m

)
install(TARGETS dnn  DESTINATION lib/ COMPONENT)

install(
    DIRECTORY 
    ${PROJECT_SOURCE_DIR}
    DESTINATION 
    include
    FILES_MATCHING 
    PATTERN "*.h" 
    PATTERN "*.hpp"
)
install(
    DIRECTORY 
    ${CMAKE_BINARY_DIR}/dnn/protos 
    DESTINATION 
    include/dnn
    FILES_MATCHING 
    PATTERN "*.h" 
    PATTERN "*.hpp"
)
