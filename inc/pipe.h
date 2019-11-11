//
// Created by viniavskyi on 11.11.19.
//

#ifndef MYSHELL_PIPE_H
#define MYSHELL_PIPE_H

#include "Command.h"
#include "environment_variables.h"
#include <vector>
#include <string>

using func_t=std::function<int(char **)>;

std::vector<Command> pipe_parser(std::string &&command, environment_variables &env);

int pipe_exec(std::vector<Command> &commands, std::map<std::string, func_t> &callbacks);

#endif //MYSHELL_PIPE_H
