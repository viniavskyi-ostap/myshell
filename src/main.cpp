//
// Created by viniavskyi on 18.10.19.
//

#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sstream>
#include <string.h>
#include <filesystem>


int main(int argc, char *argv[], char *envp[]) {
    std::stringstream ss;
    ss << "PATH=" << std::getenv("PATH") << ":" << "/home/viniavskyi/Documents/myshell/";
    const std::string tmp_string = ss.str();
    char* pString = new char[tmp_string.length() + 1];
    std::copy(tmp_string.c_str(), tmp_string.c_str() + tmp_string.length() + 1, pString);
    putenv(pString);

    char *cargs[4];
    cargs[0] = "mycat";
    cargs[1] = "myshell";
    cargs[2] = "-A";
    cargs[3] = nullptr;

    pid_t pid = fork();
    if (pid == -1) {
        printf("Can't fork");
    } else if (pid == 0) {
        std::cout << "before\n";
        execve("./mycat", cargs, NULL);
        std::cout << "after\n";
        _exit(0);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
    return 0;
}