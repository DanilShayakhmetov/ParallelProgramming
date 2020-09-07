cmake_minimum_required(VERSION 3.17)
project(lab_second)

set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/include/mpi/)

set(CMAKE_C_COMPILER "/usr/local/bin/mpicc")
set(CMAKE_CXX_COMPILER "/usr/local/bin/mpic++")


add_executable(lab_second main.cpp solution.cpp)