//
// Created by andriiprysiazhnyk on 10/18/19.
//

#include <cstring>
#include "environment_variables.h"

environment_variables::environment_variables(char **global_env) {
    int i = 0;

    while (global_env[i]) {
        int len = strlen(global_env[i]);
        char *variable = new char[len + 1];
        std::copy(global_env[i], global_env[i] + len + 1, variable);
        env.push_back(variable);
        ++i;
    }

    env.push_back(nullptr);
}

environment_variables::~environment_variables() {
    for (size_t i = 0; i < env.size() - 1; ++i)
        delete[] env[i];
}

char **environment_variables::to_array() {
    if (env.empty()) return nullptr;
    else return env.data();
}

int environment_variables::index(const std::string &key) const {
    for (size_t i = 0; i < env.size() - 1; ++i) {
        std::string current(env[i]);
        if (current.rfind(key, 0) == 0)
            return i;
    }

    return -1;
}

void environment_variables::set(const std::string &key, const std::string &value) {
    char *new_record = new char[key.size() + value.size() + 2];
    // fill buffer with key=value
    std::copy(&key[0], &key[0] + key.size(), new_record);
    new_record[key.size()] = '=';
    std::copy(&value[0], &value[0] + value.size() + 1, new_record + key.size() + 1);
    int i = index(key);
    if (i == -1) {
        env[env.size() - 1] = new_record;
        env.push_back(nullptr);
    } else {
        delete[] env[i];
        env[i] = new_record;
    }
}
