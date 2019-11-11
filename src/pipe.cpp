#include "pipe.h"
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>


std::vector<Command> pipe_parser(std::string &&command, environment_variables &env) {
    std::vector<std::string> commands_strings;
    std::vector<Command> commands;
    boost::split(commands_strings, command, boost::is_any_of("|"));
    for (size_t i = 0; i < commands_strings.size() - 1; ++i) {
        commands.emplace_back(commands_strings[i], env, true);
    }
    commands.emplace_back(commands_strings.back(), env, false);
    return commands;
}

int pipe_exec(std::vector<Command> &commands, std::map<std::string, func_t> &callbacks) {
    int prev_r = -1;
    for (size_t i = 0; i < commands.size() - 1; ++i) {
        int p[2];
        while (pipe(p) == -1) {}
        commands[i].set_fd(1, p[1]);
        commands[i + 1].set_fd(0, p[0]);
        commands[i].execute_me(p[0], callbacks);
        close(p[1]);
        close(prev_r);
        prev_r = p[0];
    }
    return commands.back().execute_me(-1, callbacks);
}