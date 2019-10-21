cmake_minimum_required(VERSION 3.14)
project(myshell)

set(CMAKE_CXX_STANDARD 17)

set(headers inc/environment_variables.h)
set(sources src/main.cpp src/environment_variables.cpp inc/parser.h src/parser.cpp inc/builtins.h src/builtins.cpp)

find_library(Readline_LIBRARY NAMES readline)
find_package(Boost REQUIRED COMPONENTS filesystem)

include_directories(inc ${Boost_INCLUDE_DIRS})

add_executable(myshell ${sources} ${headers})
target_link_libraries(myshell ${Boost_LIBRARIES} readline)
