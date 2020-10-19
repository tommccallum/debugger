#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <sys/types.h>

void run_target(const char* programname);
void run_debugger(pid_t child_pid);

#endif /* DEBUGGER_HPP */
