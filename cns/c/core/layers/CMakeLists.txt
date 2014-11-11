cmake_minimum_required(VERSION 2.8)

# Define the files we need to compile.
# Anything not in this list will not be compiled into SNN_SIM.
set(SOURCES
adapt.c
adapt.h
neuron_funcs.c
neuron_funcs.h
poisson.c
poisson.h
wtaadapt.c
wtaadapt.h
wta.c
wta.h
)

# add directory name to sources
set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()
# Append sources (with directory name) to list of all SNN_SIM sources (used at
# the parent scope).
set(SNN_SIM_SRCS ${SNN_SIM_SRCS} ${DIR_SRCS} PARENT_SCOPE)
