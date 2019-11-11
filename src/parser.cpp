//
// Created by andriiprysiazhnyk on 10/19/19.
//

#include <iostream>
#include <vector>
#include <algorithm>
#include "parser.h"
#include <boost/filesystem.hpp>

namespace bf=boost::filesystem;

static std::vector<std::string> get_tokens(std::string &command);

static int retrieve_arguments(std::string &token,
                              std::vector<char *> &arguments, int index);

static std::string substitute_variables(std::string &token);

static bool detect_wildcard(std::string &token);

static int resolve_wildcards(std::string &token, std::vector<char *> &arguments);

static bool match_pattern(const std::string &mask, const std::string &token);

static char *string_to_char_array(std::string token);

std::vector<char *> parse_command(std::string &command, int &err_code) {
    auto tokens = get_tokens(command);
    std::vector<char *> arguments;
    for (size_t i = 0; i < tokens.size(); ++i) {
        auto token = tokens[i];
        int return_status = retrieve_arguments(token, arguments, i);
        if (return_status != 0) {
            err_code = return_status;
            release_arguments(arguments);
            arguments.clear();
            break;
        }
    }
    arguments.push_back(nullptr);
    return arguments;
}

static std::vector<std::string> get_tokens(std::string &command) {
    std::vector<std::string> tokens;

    int start = -1;
    bool open_quotes = false;

    for (size_t i = 0; i < command.size(); ++i) {
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

        for (size_t i = end_index + 1; i < token.size(); ++i)
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

static int retrieve_arguments(std::string &token, std::vector<char *> &arguments, int index) {
    token.erase(std::remove(token.begin(), token.end(), '\"'), token.end());
    token = substitute_variables(token);
    if (index > 0) {
        if (detect_wildcard(token)) return resolve_wildcards(token, arguments);
    }
    if (!token.empty()) {
        arguments.push_back(string_to_char_array(token));
    }
    return 0;
}

static bool detect_wildcard(std::string &token) {
    bool opened = false;
    for (auto it = token.rbegin(); *it != '/' && it < token.rend(); ++it) {
        if (*it == '*') return true;
        if (*it == '[' && opened == true) return true;
        if (*it == '?') return true;
        if (*it == ']') opened = true;
    }
    return false;
}

static bool element_match(char c, std::string &mask_element) {
    if (mask_element.size() == 1) {
        return mask_element[0] == '?' || mask_element[0] == '*' || mask_element[0] == c;
    }
    return mask_element.substr(1, mask_element.size() - 2).find(c) != std::string::npos;
}

static bool match_pattern(const std::string &mask, const std::string &token) {
    // if mask="a*[abc]" mask_elements will contain "a", "*", "[abc]" values
    std::vector<std::string> mask_elements;
    ssize_t start = -1;
    for (size_t i = 0; i < mask.size(); ++i) {
        if (start == -1) {
            if (mask[i] == '[') start = i;
            else mask_elements.push_back(std::string(1, mask[i]));
        } else if (mask[i] == ']') {
            mask_elements.push_back(mask.substr(start, i - start + 1));
            start = -1;
        }
    }

    bool array[token.size() + 1][mask_elements.size() + 1];

    // fill array with false values
    for (size_t i = 0; i <= token.size(); ++i) {
        for (size_t j = 0; j <= mask_elements.size(); ++j) array[i][j] = false;
    }
    array[0][0] = true;

    // dynamic programming
    for (size_t i = 1; i <= token.size(); ++i) {
        for (size_t j = 1; j <= mask_elements.size(); ++j) {
            if (array[i - 1][j - 1] && element_match(token[i - 1], mask_elements[j - 1])) {
                array[i][j] = true;
            } else if (mask_elements[j - 1][0] == '*') {
                array[i][j] = array[i - 1][j] || array[i][j - 1];
            }
        }
    }

    return array[token.size()][mask_elements.size()];
}


static int resolve_wildcards(std::string &token, std::vector<char *> &arguments) {
    bool arguments_changed = false;
    // Step one : extract from token directory path
    bf::path path{token};
    auto directory = path.parent_path();

    if (directory.empty()) {
        directory = bf::path(".");
    }

    if (bf::is_directory(directory) == false) {
        std::cerr << "Directory path is invalid." << std::endl;
        return 1;
    }

    // Step two : extract from token mask
    auto mask = path.filename().string();
    // Step three: directory iteration
    bf::directory_iterator it{directory};
    while (it != bf::directory_iterator{}) {
        if (match_pattern(mask, it->path().filename().string())) {
            arguments_changed = true;
            arguments.push_back(string_to_char_array(it->path().string()));
        }
        ++it;
    }
    if (arguments_changed == false) {
        std::cerr << "No files matched specified pattern." << std::endl;
        return 2;
    }
    return 0;
}

static char *string_to_char_array(std::string token) {
    char *argument = new char[token.size() + 1];
    std::copy(&token[0], &token[0] + token.size() + 1, argument);
    return argument;
}

void release_arguments(std::vector<char *> &arguments) {
    for (auto arg: arguments)
        delete[] arg;
}