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
#include <map>
#include "parser.h"
#include "builtins.h"
#include "readline/history.h"
#include "readline/readline.h"


void add_external_programs_to_path(std::string program);

int assign_command(std::vector<char *> &arguments, char *position);

int external_command_execution(std::vector<char *> &arguments, environment_variables &env);

typedef std::function<int(char **)> func_t;

std::map<std::string, func_t> get_callbacks(int &status, environment_variables &env, std::string &current_path);


int main(int argc, char *argv[], char *envp[]) {
//    std::string current_path = "my_path";
//    char *buf;
//    while ((buf = readline( (current_path + "$ ").c_str() )) != nullptr ) {
//        if (strlen(buf) > 0) {
//            add_history(buf);
//        }
//    }

    add_external_programs_to_path(std::string(argv[0]));
//    create copy of environment variables
    environment_variables env(envp);
//    interpreter loop
    std::string current_path = boost::filesystem::current_path().string();
    int err_code = 0;
    std::map<std::string, func_t> callbacks = get_callbacks(err_code, env, current_path);

    char *buf;
    while ((buf = readline( (current_path + "$ ").c_str() )) != nullptr ) {
        if (strlen(buf) > 0) {
            add_history(buf);
        }

//        read the command and parse it
        auto arguments = parse_command(std::string(buf));

//        execute command
        if (arguments[0] == nullptr)
            continue;
        char *assignment_pos;
        if ((assignment_pos = strchr(arguments[0], '='))) { // execute assignment command
            err_code = assign_command(arguments, assignment_pos);
        } else if (callbacks.find(std::string(arguments[0])) != callbacks.end()) {
            err_code = callbacks[arguments[0]](arguments.data());
        } else {
            err_code = external_command_execution(arguments, env);
        }
        release_arguments(arguments);
    }

    delete[] buf;
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

int assign_command(std::vector<char *> &arguments, char *position) {
    if (arguments.size() > 2) {
        std::cout << "Assignment operation doesn't accept flags" << std::endl;
        return 1;
    }

    auto program_name = arguments[0];
    auto program_name_str = std::string(program_name);
    ptrdiff_t pos = position - program_name;
    setenv(program_name_str.substr(0, pos).c_str(),
            program_name_str.substr(pos + 1).c_str(), 1);
    return 0;
}

int external_command_execution(std::vector<char *> &arguments, environment_variables &env) {
    auto program_name = arguments[0];
    int status = 0;

    pid_t pid = fork();
    if (pid == -1) {
        std::cout << "Can not fork to execute command" << std::endl;
        return 1;
    } else if (pid == 0) { // child
        if (strchr(program_name, '/')) {
            execve(program_name, arguments.data(), env.to_array());
        } else {
            execvpe(program_name, arguments.data(), env.to_array());
        }
        std::cout << "Command '" << program_name << "' not found." << std::endl;
        exit(EXIT_FAILURE);
    } else { // parent
        waitpid(pid, &status, 0);
    }

    return WEXITSTATUS(status);
}

std::map<std::string, func_t> get_callbacks(int &status, environment_variables &env, std::string &current_path) {
    std::map<std::string, func_t> callbacks;

    callbacks["mexit"] = &mexit;
    callbacks["mecho"] = &mecho;

    auto bind_merrno = [&status](char **argv) {return merrno(argv, status);};
    callbacks["merrno"] = bind_merrno;

    auto bind_mpwd = [&current_path](char **argv) {return mpwd(argv, current_path);};
    callbacks["mpwd"] = bind_mpwd;

    auto bind_mcd = [&current_path](char **argv) {return mcd(argv, current_path);};
    callbacks["mcd"] = bind_mcd;

    auto bind_mexport = [&env](char **argv) {return mexport(argv, env);};
    callbacks["mexport"] = bind_mexport;

    return callbacks;
}
