cmake_minimum_required(VERSION 2.8)

## recurse
set(DIRS
    rank_out
    sgd
)

foreach(dir ${DIRS})
    add_subdirectory(${dir})
endforeach()

set(MLPACK_SRCS ${MLPACK_SRCS} ${DIR_SRCS} PARENT_SCOPE)
