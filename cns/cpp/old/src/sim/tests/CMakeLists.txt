cmake_minimum_required(VERSION 2.8)

set(SOURCES
	rand_tests.cpp
	axot_test.cpp
)

add_executable(sim_test
    test_main.cpp    
)
# Link dependencies of test executable.
target_link_libraries(sim_test
  sim_prj
  ${BOOST_unit_test_framework_LIBRARY}
)

# Copy test data into right place.
#add_custom_command(TARGET sim_test
#  POST_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data/
#      ${PROJECT_BINARY_DIR}
#)
#add_custom_command(TARGET sim_test
#  POST_BUILD
#  COMMAND ${CMAKE_COMMAND} -E tar xjpf mnist_first250_training_4s_and_9s.tar.bz2
#  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
#)
