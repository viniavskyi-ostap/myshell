//
// Created by kwh44 on 11/11/19.
//

#include "Command.h"

int assign_command(std::vector<char *> &arguments, char *position) {
    if (arguments.size() > 2) {
        std::cout << "Assignment operation doesn't accept flags" << std::endl;
        return 1;
    }

    auto program_name = arguments[0];
    auto program_name_str = std::string(program_name);
    ptrdiff_t pos = position - program_name;
    setenv(program_name_str.substr(0, pos).c_str(),
           program_name_str.substr(pos + 1).c_str(), 1);
    return 0;
}
