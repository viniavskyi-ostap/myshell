cmake_minimum_required(VERSION 3.0)
project(myshell)

set(CMAKE_CXX_STANDARD 17)
set(myshell_headers inc/environment_variables.h inc/parser.h inc/builtins.h inc/Command.h inc/pipe.h )
set(myshell_sources src/main.cpp src/environment_variables.cpp  src/parser.cpp src/builtins.cpp  src/pipe.cpp src/Command.cpp)

# add_compile_options("-DPRINT_OPTIONS")

find_library(Readline_LIBRARY NAMES readline)
find_package(Boost REQUIRED COMPONENTS filesystem program_options regex)

include_directories(inc ${Boost_INCLUDE_DIRS})

add_executable(myshell ${myshell_sources} ${myshell_headers})
target_link_libraries(myshell ${Boost_LIBRARIES} readline)

add_executable(mycat src/external_builtins/mycat.c)
set_target_properties(mycat PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/builtins")

add_executable(myls src/external_builtins/myls.cpp)
set_target_properties(myls PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/builtins" LINK_LIBRARIES "Boost::program_options;Boost::filesystem")
