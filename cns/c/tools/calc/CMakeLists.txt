cmake_minimum_required(VERSION 2.8)

# Define the files we need to compile
# Anything not in this list will not be compiled into the output library
# Do not include test programs here
set(SOURCES
    calc.c
)

# executable
add_executable(snn_calc
    snn_calc.c
    ${SOURCES}
)


# link dependencies of executable
target_link_libraries(snn_calc
  snn_sim
  m
)
install(TARGETS RUNTIME DESTINATION bin)
