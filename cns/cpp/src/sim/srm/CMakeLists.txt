cmake_minimum_required(VERSION 2.8)

set(SOURCES
	srm.h
#	srm.cpp
)

add_executable(srm_sim
    srm_main.cpp    
)
add_executable(srm_test
    srm_test.cpp    
)
# Link dependencies of test executable.
target_link_libraries(srm_sim
  sim_prj
  ${BOOST_unit_test_framework_LIBRARY}
)
target_link_libraries(srm_test
  sim_prj
  ${BOOST_unit_test_framework_LIBRARY}
)
