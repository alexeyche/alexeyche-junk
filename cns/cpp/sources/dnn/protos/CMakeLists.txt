file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${ProtoFiles})

set(ProtobufIncludePath ${CMAKE_CURRENT_BINARY_DIR}
        CACHE INTERNAL "Path to generated protobuf files.")

add_library(protos ${ProtoFiles} ${PROTO_SRCS} ${PROTO_HDRS})

SET_TARGET_PROPERTIES(protos PROPERTIES COMPILE_FLAGS "-fPIC")

target_link_libraries(protos
    ${PROTOBUF_LIBRARY}
)
