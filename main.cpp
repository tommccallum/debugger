#include <iostream>
#include <cstdarg>
#include <cstdlib>
#include <csignal>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>      // includes pid_t
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>         // POSIX standard API
#include <cerrno>           // POSIX error exit codes
#include "debugger.hpp"

// Entry point into debugger
int main(int argc, char** argv)
{
    pid_t child_pid;

    if (argc < 2) {
        std::cerr << "Expected a program name as argument\n";
        return -1;
    }

    child_pid = fork();
    if (child_pid == 0) {
        pid_t parent_pid = getpid();
        std::cout << "Parent Process: " << parent_pid << '\n';
        run_target(argv[1]);
    }
    else if (child_pid > 0) {
        std::cout << "Child Process: " << child_pid << '\n';
        run_debugger(child_pid);
    }
    else {
        perror("fork");
        return -1;
    }

    return 0;
}
