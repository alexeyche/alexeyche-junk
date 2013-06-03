cmake_minimum_required(VERSION 2.6)

set(DIRS
    socket
    )


add_executable(sim main.cpp
            connection.cpp
            main.cpp
            neurons.cpp
            poisson.cpp
            simenv.cpp)
set(SOURCES connection.cpp
            main.cpp
            neurons.cpp
            poisson.cpp
            simenv.cpp)

target_link_libraries(sim
  ${ARMADILLO_LIBRARIES}
)

