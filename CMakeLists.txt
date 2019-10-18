cmake_minimum_required(VERSION 3.14)
project(myshell)

set(headers)
set(sources src/main.cpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(myshell ${sources})