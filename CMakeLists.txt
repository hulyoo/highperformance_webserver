cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project(webserver)

add_definitions(-D_GNU_SOURCE)
add_subdirectory(src)
