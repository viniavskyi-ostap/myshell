//
// Created by andriiprysiazhnyk on 10/21/19.
//

#ifndef MYSHELL_BUILTINS_H
#define MYSHELL_BUILTINS_H

#include <string>
#include "environment_variables.h"

int merrno(char **argv, int error_code);

int mpwd(char **argv, const std::string &current_path);

int mcd(char **argv, std::string &current_path);

int mexit(char **argv, char* buf);

int mecho(char **argv);

int mexport(char **argv, environment_variables &env);

#endif //MYSHELL_BUILTINS_H
