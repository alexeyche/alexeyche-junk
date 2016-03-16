
add_executable(example
    example.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/ego)
target_link_libraries(example ${PROTOBUF_LIBRARY} dnn m)

install(TARGETS RUNTIME DESTINATION bin)
