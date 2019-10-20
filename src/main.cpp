//
// Created by viniavskyi on 18.10.19.
//

#include <iostream>
#include <environment_variables.h>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include "parser.h"


void add_external_programs_to_path(std::string program);


int main(int argc, char *argv[], char *envp[]) {
    add_external_programs_to_path(std::string(argv[0]));
    environment_variables env(envp);
    return 0;
}

void add_external_programs_to_path(std::string program) {
    boost::filesystem::path global_program_path;

    if (program[0] != '.') {
        global_program_path = boost::filesystem::path(program);
    } else {
        global_program_path = boost::filesystem::current_path();
        global_program_path /= program.substr(1, program.size() - 1);
    }

    std::string internal_programs_folder = "builtins";
    std::string internal_programs_path = (global_program_path.parent_path() / internal_programs_folder).string();

    std::string new_path = std::string(getenv("PATH")) + ":" + internal_programs_path;
    setenv("PATH", new_path.c_str(), 1);
}
