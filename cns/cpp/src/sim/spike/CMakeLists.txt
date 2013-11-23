cmake_minimum_required(VERSION 2.8)

set(SOURCES
    lzw.c
    lzw.cpp
)

# add directory name to sources
set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()

add_executable(lzw_test_cpp
    lzw_test.cpp
)

add_executable(lzw_test_c
    lzw_test.c
)
# Link dependencies of test executable.
target_link_libraries(lzw_test_cpp
  sim_prj
  ${BOOST_unit_test_framework_LIBRARY}
)
target_link_libraries(lzw_test_c
  sim_prj
  ${BOOST_unit_test_framework_LIBRARY}
)
set(SIM_SRCS ${SIM_SRCS} ${DIR_SRCS} PARENT_SCOPE)
