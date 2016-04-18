
add_executable(example
    example.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/ego)
target_link_libraries(example ${PROTOBUF_LIBRARY} dnn m)

get_filename_component(model_bn ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)
install(TARGETS example RUNTIME DESTINATION models/${model_bn})

file(GLOB ConfigFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.pb.txt")

foreach(ConfigFile ${ConfigFiles})
	message(STATUS ${ConfigFile})
	install(CODE
	    "EXECUTE_PROCESS(COMMAND cp -f ${ConfigFile} ${CMAKE_INSTALL_PREFIX}/models/${model_bn})"
	)
endforeach(ConfigFile)
