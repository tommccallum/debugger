#include <map>
#include <vector>
#include <cstdint>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <algorithm>

using memory_address = uintptr_t;
using cpu_instruction = uintptr_t;

enum class BreakpointStatus {
    NOT_SET = 0,
    SET = 1
};

struct Breakpoint 
{
    pid_t           process = 0;
    memory_address  address = 0;
    cpu_instruction instruction = 0;
    BreakpointStatus status = BreakpointStatus::NOT_SET;
};

std::vector<Breakpoint> breakpoints;

void add_breakpoint(memory_address addr) {
    pid_t child_pid = getpid();
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child_pid, 0, &regs);

    // get the data current in the spot where we want to put our breakpoint
    uintptr_t data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*) addr, 0 );
    
    // add our INT 3 signal to create breakpoint
    uintptr_t data_with_trap = (data & 0xFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT, child_pid, (void*)addr, (void*)data_with_trap);
    breakpoints.emplace_back( child_pid, addr, data, BreakpointStatus::SET );
}

void stop_breakpoint(pid_t process, memory_address address) {
    auto it = std::find_if(breakpoints.begin(), breakpoints.end(), 
        [&process, &address](auto b) { return b.process == process && b.address == address; } );
    if ( it != breakpoints.end() ) {
        ptrace(PTRACE_POKETEXT, it->process, (void*)it->address, (void*)it->instruction);
        struct user_regs_struct regs;
        ptrace(PTRACE_GETREGS, it->process, 0, &regs);
        regs.rip -= 1;
        ptrace(PTRACE_SETREGS, it->process, 0, &regs);
        it->status = BreakpointStatus::NOT_SET;
    }
}

void remove_breakpoint(pid_t process, memory_address address ) {
    auto it = std::find_if(breakpoints.begin(), breakpoints.end(), 
        [&process, &address](auto b) { return b.process == process && b.address == address; } );
    if ( it->status == BreakpointStatus::SET ) {
        stop_breakpoint(process, address);
    }
    std::remove(breakpoints.begin(), breakpoints.end(), *it);
}

int continue_to_first_instruction() {
    int wait_status;
    wait(&wait_status);
    return wait_status;
}

int continue_to_next_signal(pid_t process) {
    int wait_status;
    ptrace(PTRACE_CONT, process, 0, 0);
    wait(&wait_status);
    return wait_status;
}


