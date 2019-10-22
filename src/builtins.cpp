//
// Created by andriiprysiazhnyk on 10/21/19.
//

#include <iostream>
#include <cstring>
#include <string>
#include "builtins.h"

static int get_argc(char **argv){
    int i = 0;
    for(; argv[i]; ++i);
    return i;
}

int mexit(char **argv) {
    int argc = get_argc(argv);

    int exit_code = 0;
    if (argc > 1){
        for (int i = 1; i < argc; ++i){
            if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
                std::cout << "Exit with given exit code.\n"
                             "Usage: mexit [exit_code] [-h|--help]" << std::endl;
                return 0;
            }
        }
        try {
            std::cout << "here" << std::endl;
            exit_code = std::stoi(argv[1]);
        }
        catch (std::invalid_argument &e) {
            std::cerr << "mexit: Illegal number: " << argv[1] << std::endl;
            return 1;
        }
    }
    exit(exit_code);
}

int mpwd(char **argv, const std::string &current_path) {
    std::cout << current_path << std::endl;
    return 1;
}

int mcd(char **argv, std::string &current_path) {
    std::cout << "MCD" << "\n";
    return 2;
}

int mecho(char **argv) {
    std::cout << "MECHO" << "\n";
    return 4;
}

int mexport(char **argv, environment_variables &env) {
    std::cout << "MEEXPORT" << "\n";
    return 5;
}

int merrno(char **argv, int error_code) {
    int argc = get_argc(argv);
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
                std::cout << "Print return code of the last command.\n"
                             "Usage: merrno [-h|--help]" << std::endl;
                return 0;
            }
        }
    }
    std::cout << error_code << std::endl;
    return 0;
}