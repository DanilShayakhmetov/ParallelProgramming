cmake_minimum_required(VERSION 3.16)
project(lab_fourth)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenMP)


add_executable(lab_fourth main.cpp Dijkstra.cpp Graph.cpp)