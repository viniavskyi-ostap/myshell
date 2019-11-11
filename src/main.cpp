//
// Created by viniavskyi on 18.10.19.
//

#include <string>
#include <cstring>
#include <environment_variables.h>
#include <boost/filesystem.hpp>
#include <map>
#include <boost/regex.hpp>
#include <ostream>
#include "builtins.h"
#include "readline/history.h"
#include "readline/readline.h"
#include "pipe.h"


namespace bf=boost::filesystem;

void add_external_programs_to_path(std::string program);

typedef std::function<int(char **)> func_t;

std::map<std::string, func_t> get_callbacks(int &status, environment_variables &env, std::string &current_path);

class command_buffer{
public:
    char* buffer = nullptr;
    ~command_buffer(){
        delete[] buffer;
    }
};

int interpret(std::string &&command, std::map<std::string, func_t> &callbacks, environment_variables &env);

int main(int argc, char *argv[], char *envp[]) {
    add_external_programs_to_path(std::string(argv[0]));
//    create copy of environment variables
    environment_variables env(envp);
//    interpreter loop
    std::string current_path = boost::filesystem::current_path().string();
    int err_code = 0;
    auto buf = command_buffer();
    std::map<std::string, func_t> callbacks = get_callbacks(err_code, env, current_path);

    while ((buf.buffer = readline((current_path + "$ ").c_str())) != nullptr) {
        if (strlen(buf.buffer) > 0) {
            add_history(buf.buffer);
        }
        err_code = interpret(std::string(buf.buffer), callbacks, env);
    }

    return 0;
}

void add_external_programs_to_path(std::string program) {
    boost::filesystem::path global_program_path;

    if (program[0] == '/') {
        global_program_path = boost::filesystem::path(program);
    } else {
        global_program_path = boost::filesystem::current_path();
        if (program[0] == '.')
            global_program_path /= program.substr(1, program.size() - 1);
        else
            global_program_path /= program;
    }

    std::string internal_programs_folder = "builtins";
    std::string internal_programs_path = (global_program_path.parent_path() / internal_programs_folder).string();

    std::string new_path = std::string(getenv("PATH")) + ":" + internal_programs_path;
    setenv("PATH", new_path.c_str(), 1);
}


std::map<std::string, func_t> get_callbacks(int &status, environment_variables &env, std::string &current_path) {
    std::map<std::string, func_t> callbacks;

    callbacks["mecho"] = &mecho;
    callbacks["mexit"] = &mexit;

    auto bind_merrno = [&status](char **argv) { return merrno(argv, status); };
    callbacks["merrno"] = bind_merrno;

    auto bind_mpwd = [&current_path](char **argv) { return mpwd(argv, current_path); };
    callbacks["mpwd"] = bind_mpwd;

    auto bind_mcd = [&current_path](char **argv) { return mcd(argv, current_path); };
    callbacks["mcd"] = bind_mcd;

    auto bind_mexport = [&env](char **argv) { return mexport(argv, env); };
    callbacks["mexport"] = bind_mexport;

    return callbacks;
}

int interpret(std::string &&command, std::map<std::string, func_t> &callbacks, environment_variables &env){
//    find # symbol
    auto comment_sign_pos = command.find('#');
    if (comment_sign_pos != std::string::npos) {
        command = command.substr(0, comment_sign_pos);
    }

    const std::string temp_file = ".temp";
    const boost::regex regex("\\$(.*)");
    boost::sregex_token_iterator iter(command.begin(), command.end(), regex, 0);
    boost::sregex_token_iterator end;

    for (; iter != end; ++iter) {
        std::string command_to_execute = (*iter).str().substr(2, (*iter).str().size() - 3) + " > " + temp_file;
        pipe_exec(command_to_execute, callbacks, env);
        std::ifstream in(temp_file);
        auto ss = std::ostringstream{};
        ss << in.rdbuf();
        auto output = ss.str();
        command = boost::regex_replace(command, regex, output, boost::format_first_only);
    }
    std::cout << command << "\n";
    return pipe_exec(command, callbacks, env);
}
