//
// Created by viniavskyi on 18.10.19.
//

#include <iostream>
#include <string>
#include <cstring>
#include <environment_variables.h>
#include <boost/filesystem.hpp>
#include <unistd.h>
#include <sys/wait.h>
#include "parser.h"


void add_external_programs_to_path(std::string program);


int main(int argc, char *argv[], char *envp[]) {
    add_external_programs_to_path(std::string(argv[0]));
//    create copy of environment variables
    environment_variables env(envp);
//    interpreter loop
    std::string command;
    std::string current_path = boost::filesystem::current_path().string();
    while (true){
//        read the command and parse it
        std::cout << current_path << "$ ";
        std::getline(std::cin, command);
        auto arguments = parse_command(command);
//        execute command
        if (arguments[0] == nullptr)
            continue;
        pid_t pid = fork();
        if (pid == -1){
            std::cout << "Can not fork to execute command" << std::endl;
        } else if (pid == 0){ // child
            auto program_name = arguments[0];
            if (strchr(program_name, '/')){
                execve( program_name, arguments.data(), env.to_array());
            } else{
                execvpe(program_name, arguments.data(), env.to_array());
            }
            std::cout << "Command '" << program_name << "' not found." << std::endl;
            return -1;
        } else{ // parent
            int status;
            waitpid(pid, &status, 0);
            std::cout << "Status: " << status << std::endl;
        }
        release_arguments(arguments);
    }
    return 0;
}

void add_external_programs_to_path(std::string program) {
    boost::filesystem::path global_program_path;

    if (program[0] == '/') {
        global_program_path = boost::filesystem::path(program);
    } else {
        global_program_path = boost::filesystem::current_path();
        if (program[0] == '.')
            global_program_path /= program.substr(1, program.size() - 1);
        else
            global_program_path /= program;
    }

    std::string internal_programs_folder = "builtins";
    std::string internal_programs_path = (global_program_path.parent_path() / internal_programs_folder).string();

    std::string new_path = std::string(getenv("PATH")) + ":" + internal_programs_path;
    setenv("PATH", new_path.c_str(), 1);
}
