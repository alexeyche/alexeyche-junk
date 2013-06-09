cmake_minimum_required(VERSION 2.8)

# Define the files we need to compile
# Anything not in this list will not be compiled into the output library
# Do not include test programs here
set(SOURCES
        basic_elements.h 
        connection.cpp
        connection.h
        neurons.cpp
        neurons.h
        poisson.cpp
        poisson.h
        simenv.cpp
        simenv.h
        stat_collector.h
        axon.h
        synapse.h
)

# add directory name to sources
set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()
# append sources (with directory name) to list of all MLPACK sources (used at the parent scope)
set(SIM_SRCS ${SIM_SRCS} ${DIR_SRCS} PARENT_SCOPE)

# executable
add_executable(sim
  main.cpp
)
# link dependencies of executable
target_link_libraries(sim
  sim_prj
)
install(TARGETS sim RUNTIME DESTINATION bin)
