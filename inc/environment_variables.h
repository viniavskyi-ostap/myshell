//
// Created by andriiprysiazhnyk on 10/18/19.
//

#ifndef MYSHELL_ENVIRONMENT_VARIABLES_H
#define MYSHELL_ENVIRONMENT_VARIABLES_H

#include <string>
#include <vector>


class environment_variables {
private:
    std::vector<char *> env;

    int index(const std::string &key) const;

public:
    explicit environment_variables(char **global_env);
    ~environment_variables();


    void set(const std::string &key, const std::string &value);

    char **to_array();
};

#endif //MYSHELL_ENVIRONMENT_VARIABLES_H
