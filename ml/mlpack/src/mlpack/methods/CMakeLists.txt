cmake_minimum_required(VERSION 2.8)

## recurse
set(DIRS
  det
  emst
  gmm
  hmm
  kernel_pca
  kmeans
  lars
  linear_regression
  local_coordinate_coding
  mvu
  naive_bayes
  nca
  neighbor_search
  nmf
  pca
  radical
  range_search
  sparse_coding
)

foreach(dir ${DIRS})
    add_subdirectory(${dir})
endforeach()

set(MLPACK_SRCS ${MLPACK_SRCS} ${DIR_SRCS} PARENT_SCOPE)
