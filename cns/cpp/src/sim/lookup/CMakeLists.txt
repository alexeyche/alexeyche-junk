cmake_minimum_required(VERSION 2.8)

#set(SOURCES
#	generate_table.cpp
#)

add_executable(gen_table
    generate_table.cpp    
)
# Link dependencies of test executable.
target_link_libraries(gen_table
  sim_prj
  ${BOOST_unit_test_framework_LIBRARY}
)
