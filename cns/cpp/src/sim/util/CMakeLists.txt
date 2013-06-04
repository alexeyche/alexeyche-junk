cmake_minimum_required(VERSION 2.8)

set(SOURCES
  cli.hpp
  cli.cpp
  cli_deleter.hpp
  cli_deleter.cpp
  cli_impl.hpp
  log.hpp
  log.cpp
  nulloutstream.hpp
  option.hpp
  option.cpp
  option_impl.hpp
  prefixedoutstream.hpp
  prefixedoutstream.cpp
  prefixedoutstream_impl.hpp
  rand_funcs.hpp
  timers.hpp
  timers.cpp
)

# add directory name to sources
set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()

set(SIM_SRCS ${SIM_SRCS} ${DIR_SRCS} PARENT_SCOPE)
