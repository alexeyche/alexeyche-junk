cmake_minimum_required(VERSION 2.8)
project(snn_sim C CXX)

set(SNNSIM_MAJOR_VERSION 0)
set(SNNSIM_MINOR_VERSION 1)
set(SNNSIM_PATCH_VERSION 0)
set(SNNSIM_VERSION
  ${SNNSIM_MAJOR_VERSION}.${SNNSIM_MINOR_VERSION}.${SNNSIM_PATCH_VERSION})
 
# First, define all the compilation options.
# We default to debugging mode for developers.
option(DEBUG "Compile with debugging information" OFF)
option(PROFILE "Compile with profiling information" OFF)

# Set the CFLAGS and CXXFLAGS depending on the options the user specified.
# Only GCC-like compilers support -Wextra, and other compilers give tons of
# output for -Wall, so only -Wall and -Wextra on GCC.
#if(CMAKE_COMPILER_IS_GNUCC)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
#  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
#endif(CMAKE_COMPILER_IS_GNUCC)

find_package (Threads)

# Debugging CFLAGS.  Turn optimizations off; turn debugging symbols on.
if(DEBUG)
  add_definitions(-DDEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -g -O0")
else()
  add_definitions(-DNDEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -O3")
endif(DEBUG)

# Profiling CFLAGS.  Turn profiling information on.
if(PROFILE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
endif(PROFILE)

# set path right
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


include_directories(${CMAKE_SOURCE_DIR})

# Recurse into the rest of the project.
add_subdirectory(snnlib)
add_subdirectory(tools)

