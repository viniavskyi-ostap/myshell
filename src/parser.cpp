//
// Created by andriiprysiazhnyk on 10/19/19.
//

#include <iostream>
#include <vector>
#include <algorithm>
#include "parser.h"

static std::vector<std::string> get_tokens(std::string &command);

static void retrieve_arguments(std::string &token,
                               std::vector<char *> &arguments); // TODO: wildcards substitution
static std::string substitute_variables(std::string &token);

std::vector<char *> parse_command(std::string command) {
    auto tokens = get_tokens(command);
    std::vector<char *> arguments;

    for (auto &token: tokens) {
        retrieve_arguments(token, arguments);
    }

    arguments.push_back(nullptr);
    return arguments;
}

static std::vector<std::string> get_tokens(std::string &command) {
    std::vector<std::string> tokens;

    int start = -1;
    bool open_quotes = false;

    for (int i = 0; i < command.size(); ++i) {
        if (command[i] == '"') open_quotes = !open_quotes;

        if (command[i] != ' ') {
            if (start == -1) start = i;
        } else {
            if (start != -1 && !open_quotes) {
                tokens.push_back(command.substr(start, i - start));
                start = -1;
            }
        }

        if (i == command.size() - 1 && start != -1) tokens.push_back(command.substr(start, i - start + 1));
    }

    return tokens;
}

static std::string substitute_variables(std::string &token) {
    int start_index = token.find('$'), end_index;

    while (start_index != std::string::npos) {
        end_index = start_index;

        for (int i = end_index + 1; i < token.size(); ++i)
            if (token[i] != '$' && token[i] != ' ')
                ++end_index;
            else
                break;

        char *var_value = getenv(token.substr(start_index + 1, end_index - start_index).c_str());
        std::string res = var_value ? var_value : "";

        token = token.substr(0, start_index) + res + token.substr(end_index + 1, token.size() - end_index);
        start_index = token.find('$');
    }
    return token;
}

static void retrieve_arguments(std::string &token, std::vector<char *> &arguments) {
    token.erase(std::remove(token.begin(), token.end(), '\"'), token.end());
    token = substitute_variables(token);

    if (!token.empty()) {
        char *argument = new char[token.size() + 1];
        std::copy(&token[0], &token[0] + token.size() + 1, argument);
        arguments.push_back(argument);
    }
}

void release_arguments(std::vector<char *> &arguments) {
    for (auto arg: arguments)
        delete[] arg;
}