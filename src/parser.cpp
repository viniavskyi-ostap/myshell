//
// Created by andriiprysiazhnyk on 10/19/19.
//

#include <vector>
#include <algorithm>
#include "parser.h"

static std::vector<std::string> get_tokens(std::string &command);
static void retrieve_arguments(std::string &token, std::vector<char *> &arguments); // TODO: env variables and wildcards substitution

std::vector<char *> parse_command(std::string command) {
    auto tokens = get_tokens(command);
    std::vector<char *> arguments;

    for (auto& token: tokens) {
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
                tokens.push_back(command.substr(start, i - start + 1));
                start = -1;
            }
        }

        if (i == command.size() - 1 && start != -1) tokens.push_back(command.substr(start, i - start + 1));
    }

    return tokens;
}

static void retrieve_arguments(std::string &token, std::vector<char *> &arguments) {
    token.erase(std::remove(token.begin(), token.end(), '\"'), token.end());

    char *argument = new char[token.size() + 1];
    std::copy(&token[0], &token[0] + token.size() + 1, argument);
    arguments.push_back(argument);
}

void release_arguments(std::vector<char *> &arguments) {
    for (auto arg: arguments)
        delete[] arg;
}