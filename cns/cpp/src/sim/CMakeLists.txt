include_directories(..) # <sim/[whatever]>

cmake_minimum_required(VERSION 2.6)

# Add core.hpp to list of sources.
set(${SIM_SRCS} ${SIM_SRCS} "core.h")

set(DIRS
    data
    socket
    util
    oldsim
    tests
    lookup
    srm
    int
    )

foreach(dir ${DIRS})
    add_subdirectory(${dir})
endforeach()

add_library(sim_prj SHARED ${SIM_SRCS})

target_link_libraries(sim_prj
  ${ARMADILLO_LIBRARIES}
  ${Boost_LIBRARIES}
)

set_target_properties(sim_prj
  PROPERTIES
  VERSION 1.0
  SOVERSION 1
)

# Collect all header files in the library.
file(GLOB_RECURSE INCLUDE_H_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
file(GLOB_RECURSE INCLUDE_HPP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp)
set(INCLUDE_FILES ${INCLUDE_H_FILES} ${INCLUDE_HPP_FILES})

add_custom_command(TARGET sim POST_BUILD
  COMMENT "Moving header files to include/sim/"
  COMMAND ${CMAKE_COMMAND} ARGS -E
    make_directory ${CMAKE_BINARY_DIR}/include/sim/)

# Then copy each of the header files over to that directory.
foreach(incl_file ${INCLUDE_FILES})
  add_custom_command(TARGET sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} ARGS -E
      copy ${CMAKE_CURRENT_SOURCE_DIR}/${incl_file}
           ${CMAKE_BINARY_DIR}/include/sim/${incl_file})
endforeach()

# At install time, we simply install that directory of header files we
# collected to include/.
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/sim DESTINATION include)

# Set generated executables to be installed.  Unfortunately they must manually
# be entered...
install(TARGETS sim_prj
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# For 'make test'. 
add_custom_target(test 
  ${CMAKE_BINARY_DIR}/bin/sim_test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/ # This is where test files are put. 
  COMMENT "Running SIM test" 
  DEPENDS sim_test)

