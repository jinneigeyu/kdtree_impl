cmake_minimum_required(VERSION 3.10.2)

project(KDTREE)

include_directories(include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(main_exe src/main.cpp)