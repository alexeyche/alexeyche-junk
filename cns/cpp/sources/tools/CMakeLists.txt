cmake_minimum_required(VERSION 2.8)

## recurse
set(DIRS
    sim
    dumptool
    proc
    gammatone_fb
    #filt
    #postproc
    #calc
)

foreach(dir ${DIRS})
    add_subdirectory(${dir})
endforeach()

set(SNN_SIM_SRCS ${SNN_SIM_SRCS} ${DIR_SRCS} PARENT_SCOPE)
