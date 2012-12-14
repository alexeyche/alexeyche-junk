cmake_minimum_required(VERSION 2.8)

# Define the files we need to compile.
# Anything not in this list will not be compiled into MLPACK.
set(SOURCES
    turn.cpp
    scaling.cpp
    turn.hpp
    scaling.hpp
    rank_out.cpp
    rank_out.hpp
)

# Add directory name to sources.
set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()
# Append sources (with directory name) to list of all MLPACK sources (used at
# the parent scope).
set(MLPACK_SRCS ${MLPACK_SRCS} ${DIR_SRCS} PARENT_SCOPE)

# The main TURN executable.
add_executable(rank_out
  rank_out_main.cpp
)
target_link_libraries(rank_out
  mlpack
)
install(TARGETS rank_out RUNTIME DESTINATION bin)
