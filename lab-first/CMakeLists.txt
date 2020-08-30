cmake_minimum_required(VERSION 3.16)
project(lab_first)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenMP)

add_executable(lab_first main.cpp multiplier.cpp matrix.cpp)
