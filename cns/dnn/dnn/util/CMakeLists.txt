cmake_minimum_required(VERSION 2.8)

set(DIRS
    log
)

foreach(dir ${DIRS})
  add_subdirectory(${dir})
endforeach()

set(SOURCES
    any.cpp
    proto_options.cpp
    string.cpp
    parse_parenthesis.cpp
    fs.cpp
    pretty_print.cpp
    protobuf.cpp
    json.cpp
    serial.cpp
    thread.cpp
    random.cpp
)

set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()

set(DNN_SRCS ${DNN_SRCS} ${DIR_SRCS} PARENT_SCOPE)
