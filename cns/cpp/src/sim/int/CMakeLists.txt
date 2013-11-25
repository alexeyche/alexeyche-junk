cmake_minimum_required(VERSION 2.8)

set(SOURCES
	DEIntegrationConstants.h  
    DEIntegrator.h  
    gauss_legendre.c
    gauss_legendre.h
)

add_executable(int_test
main.cpp
)

# Link dependencies of test executable.
target_link_libraries(int_test
  sim_prj
  ${BOOST_unit_test_framework_LIBRARY}
)
