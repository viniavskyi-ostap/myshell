//
// Created by viniavskyi on 11.11.19.
//

#ifndef MYSHELL_COMMAND_H
#define MYSHELL_COMMAND_H
//
// Created by kwh44 on 11/8/19.
//

#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <wait.h>
#include <map>
#include <functional>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <fcntl.h>
#include "environment_variables.h"
#include "parser.h"

using func_t=std::function<int(char **)>;

int assign_command(std::vector<char *> &arguments, char *position);


class Command {
    int in_fd = 0, out_fd = 1, err_fd = 2;
    int parse_err_code = 0;
    bool background_execution = false;

    environment_variables &env;
public:
    std::vector<char *> arguments;

    Command(std::string command, environment_variables &env,
            bool background_execution) : env(env), background_execution(background_execution) {
        std::cout << command << std::endl;
        handle_redirections(command);
        arguments = parse_command(command, parse_err_code);
        for (size_t i = 0; arguments[i]; ++i) {
            std::cout << arguments[i] << std::endl;
        }
        std::cout << "in command constructor\n";
    }

    
    void set_fd(int which, int new_fd) {
        if (which == 0) {
            if (in_fd != 0) {
                close(in_fd);
            }
            in_fd = new_fd;
        } else if (which == 1) {
            if (out_fd != 1) {
                close(out_fd);
            }
            out_fd = new_fd;
        } else if (which == 2) {
            if (err_fd != 2) {
                close(err_fd);
            }
            err_fd = new_fd;
        }
    }

    int execute_me(int fd_to_close, std::map<std::string, func_t> &callbacks) {
        std::cout << " IN execute me\n";
        for (size_t i = 0; arguments[i]; ++i) {
            std::cout << arguments[i] << std::endl;
        }
        if (parse_err_code != 0) {
            return parse_err_code;
        }
        if (arguments[0] == nullptr) return 0;
        char *assignment_pos;
        if ((assignment_pos = strchr(arguments[0], '='))) { // execute assignment command
            return assign_command(arguments, assignment_pos);
        } else if (callbacks.find(std::string(arguments[0])) != callbacks.end()) {
            // execute in current process builtin command
            int out_copy = dup(1);
            dup2(out_fd, 1);
            int internal_err_code = callbacks[arguments[0]](arguments.data());
            dup2(out_copy, 1);
            return internal_err_code;
        } else {
            auto program_name = arguments[0];
            int status = 0;
            pid_t pid = fork();
            if (pid == -1) {
                std::cout << "Can not fork to execute command" << std::endl;
                return 1;
            } else if (pid == 0) { // child
                if (in_fd != 0) {
                    dup2(in_fd, 0);
                }
                if (out_fd != 1) {
                    dup2(out_fd, 1);
                }
                if (err_fd != 2) {
                    dup2(err_fd, 2);
                }

                if (fd_to_close != -1) close(fd_to_close);
                if (strchr(program_name, '/')) {
                    execve(program_name, arguments.data(), env.to_array());
                } else {
                    execvpe(program_name, arguments.data(), env.to_array());
                }
                std::cerr << "Command '" << program_name << "' not found." << std::endl;
                exit(EXIT_FAILURE);
            } else { // parent
                if (background_execution) return 0;
                else {
                    waitpid(pid, &status, 0);
                    return WEXITSTATUS(status);
                }
            }
        }

    }

    ~Command() {
        if (in_fd != 0) close(in_fd);
        if (out_fd != 1) close(out_fd);
        if (err_fd != 2) close(err_fd);
        release_arguments(arguments);
    }

private:
    void handle_redirections(std::string &command) {
//        rstrip command

        boost::algorithm::trim(command);
//        handle background execution
        if (command.length() > 1 && command.substr(command.length() - 2) == " &") {
            background_execution = true;
            command = command.substr(0, command.length() - 2);
        }

        const boost::regex regex("[0-2]?[><](&[0-2]|[\\s]*[^\\s]*)");
        boost::sregex_token_iterator iter(command.begin(), command.end(), regex, 0);
        boost::sregex_token_iterator end;

        for (; iter != end; ++iter) {
            extract_descriptors((*iter).str());
        }


        command = boost::regex_replace(command, regex, "");

    }

    void extract_descriptors(const std::string &str) {
        int fd_to_change = -1, start_index = 2;
        switch (str[0]) {
            case '0': {
                fd_to_change = 0;
                break;
            }
            case '1': {
                fd_to_change = 1;
                break;
            }
            case '2': {
                fd_to_change = 2;
                break;
            }
            case '>': {
                fd_to_change = 1;
                start_index = 1;
                break;
            }
            case '<': {
                fd_to_change = 0;
                start_index = 1;
                break;
            }
        }
        auto substr = str.substr(start_index);
        boost::algorithm::trim(substr);
        if (substr[0] == '&') {
            switch (substr[1]) {
                case '0': {
                    set_fd(fd_to_change, in_fd);
                    break;
                }
                case '1': {
                    set_fd(fd_to_change, out_fd);
                    break;
                }
                case '2': {
                    set_fd(fd_to_change, err_fd);
                    break;
                }
            }
        } else {
            int fd = open(substr.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            set_fd(fd_to_change, fd);
        }
    }

};

#endif //MYSHELL_COMMAND_H
