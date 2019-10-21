//
// Created by andriiprysiazhnyk on 10/21/19.
//

#include <iostream>
#include "builtins.h"

int mexit(char **argv) {
    exit(0);
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
    std::cout << "MERRNO: " << error_code << "\n";
    return 0;
}