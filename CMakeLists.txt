cmake_minimum_required(VERSION 3.0)
project(myshell)

set(CMAKE_CXX_STANDARD 17)

set(myshell_headers inc/environment_variables.h)
set(myshell_sources src/main.cpp src/environment_variables.cpp inc/parser.h src/parser.cpp inc/builtins.h src/builtins.cpp)

find_library(Readline_LIBRARY NAMES readline)
find_package(Boost REQUIRED COMPONENTS filesystem)

include_directories(inc ${Boost_INCLUDE_DIRS})

add_executable(myshell ${myshell_sources} ${myshell_headers})
target_link_libraries(myshell ${Boost_LIBRARIES} readline)

#set(BUILTINS_DIR builtins/)
#set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILTINS_DIR})
#add_subdirectory(${BUILTINS_DIR})
#add_executable(mycat src/external_builtins/mycat.c)