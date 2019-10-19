//
// Created by andriiprysiazhnyk on 10/19/19.
//

#ifndef MYSHELL_PARSER_H
#define MYSHELL_PARSER_H

#include <string>
#include <vector>

std::vector<char *> parse_command(std::string command);

void release_arguments(std::vector<char *> &arguments);

#endif //MYSHELL_PARSER_H
