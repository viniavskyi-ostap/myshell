
//
// Created by andriiprysiazhnyk on 10/21/19.
//

#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include "builtins.h"


int merrno(char **argv, int error_code) {
    if (argv[1] != nullptr) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
            std::cout << "merrno [-h|--help]" << std::endl;
        else return -1; // invalid option error
    } else std::cout << error_code << std::endl;
    return 0;
}

int mexit(char **argv) {
    if (argv[1] != nullptr) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            std::cout << "mexit [exit code] [-h|--help]" << std::endl;
            return 0;
        } else exit(atoi(argv[1]));
    }
    exit(0);
}

int mpwd(char **argv, const std::string &current_path) {
    if (argv[1] != nullptr) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            std::cout << "mpwd [-h|--help]" << std::endl;
            return 0;
        } else return -1; // invalid option error
    } else std::cout << current_path << std::endl;
    return 0;
}

int mcd(char **argv, std::string &current_path, const environment_variables &env) {
    if (argv[1] != nullptr) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
            std::cout << "mcd <path> [-h|--help]" << std::endl;
        else {
            int return_code;
            if (strcmp(argv[1], "~") == 0) /* change to $HOME */ {
                char home[] = {'H', 'O', 'M', 'E'};
                auto value = env.get(home);
                if (value.empty()) return -1; // HOME variable was not set
                return_code = chdir(value.c_str());
                if (return_code == 0) current_path = value;
            } else {
                char *path = realpath(argv[1], nullptr);
                return_code = chdir(path);
                if (return_code == 0) current_path = std::string(path, strlen(path));
                free(path);
            }
            return return_code;
        }
    }
    return 0;
}

int mecho(char **argv, environment_variables &env) {
    if (argv[1] != nullptr) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            std::cout << "mecho [text|$<var_name>] [text|$<var_name>]  [text|$<var_name>] ..." << std::endl;
            return 0;
        }
        for (size_t i = 1; argv[i] != nullptr; ++i) {
            if (argv[i][0] == '$') {
                auto value = env.get(argv[i] + 1);
                if (value.empty()) std::cerr << "Variable does not exist." << std::endl;
                else std::cout << value << ' ';
            } else {
                std::cout << argv[i] << ' ';
            }
        }
    }
    std::cout << std::endl;
    return 0;
}

int mexport(char **argv, environment_variables &env) {
    if (argv[1] != nullptr) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            std::cout << "mecho [text|$<var_name>] [text|$<var_name>]  [text|$<var_name>] ..." << std::endl;
            return 0;
        }
        auto key_end = strchr(argv[1], '=');
        if (key_end == nullptr) {
            auto value = env.get(argv[1]);
            if (value.empty()) env.set(std::string(argv[1]), std::string());
            return 0;
        }
        std::string key(argv[1], key_end - argv[1]), value(key_end + 1, strlen(key_end + 1));
        env.set(key, value);
    }
    return 0;
}