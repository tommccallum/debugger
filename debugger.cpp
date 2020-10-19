// https://github.com/eliben/code-for-blog/blob/master/2011/simple_tracer.c
/* Code sample: using ptrace for simple tracing of a child process.
**
** Note: this was originally developed for a 32-bit x86 Linux system; some
** changes may be required to port to x86-64.
**
** Eli Bendersky (http://eli.thegreenplace.net)
** This code is in the public domain.
*/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>
#include <cstdint>
#include <cstring>

/* Print a message to stdout, prefixed by the process ID
*/
void procmsg(const char* format, ...)
{
    va_list ap;
    fprintf(stdout, "[%d] ", getpid());
    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
}


void run_target(const char* programname)
{
    procmsg("target started. will run '%s'\n", programname);

    /* Allow tracing of this process */
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        perror("ptrace");
        return;
    }

    /* Replace this process's image with the given program */
    execl(programname, programname, 0);
}


void run_debugger(pid_t child_pid)
{
    int wait_status;
    unsigned icounter = 0;
    procmsg("debugger started\n");

    /* Wait for child to stop on its first instruction */
    wait(&wait_status);

    // set our breakpoint
    /* Obtain and show child's instruction pointer */
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
    procmsg("Child started. IP = 0x%016x\n", regs.rip);

    // look at the current data at where we want our breakpoint
    uintptr_t addr = 0x40101e;
    uintptr_t data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*) addr, 0 );
    procmsg("Original mdata at 0x%016x: 0x%016x\n", addr, data);

    /* Write the trap instruction 'int 3' into the address */
    uintptr_t data_with_trap = (data & 0xFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT, child_pid, (void*)addr, (void*)data_with_trap);

    /* See what's there again... */
    unsigned readback_data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)addr, 0);
    procmsg("After trap, data at 0x%016x: 0x%016x\n", addr, readback_data);

    /* Let the child run to the breakpoint and wait for it to reach it */
    ptrace(PTRACE_CONT, child_pid, 0, 0);

    wait(&wait_status);
    

    if (WIFSTOPPED(wait_status)) {
        procmsg("Child got a signal: %s\n", strsignal(WSTOPSIG(wait_status)));
    }
    else {
        perror("wait");
        return;
    }

    /* See where the child is now */
    ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
    procmsg("Child stopped at IP = 0x%016x\n", regs.rip);


    /* Remove the breakpoint by restoring the previous data
    ** at the target address, and unwind the EIP back by 1 to
    ** let the CPU execute the original instruction that was
    ** there.
    */
    ptrace(PTRACE_POKETEXT, child_pid, (void*)addr, (void*)data);
    regs.rip -= 1;
    ptrace(PTRACE_SETREGS, child_pid, 0, &regs);

    /* The child can continue running now */
    ptrace(PTRACE_CONT, child_pid, 0, 0);


    // while (WIFSTOPPED(wait_status)) {
    //     icounter++;
    //     struct user_regs_struct regs;
    //     ptrace(PTRACE_GETREGS, child_pid, 0, &regs);

    //     // if 32-bit then regs.eip
    //     // if 64-bit then regs.rip
    //     unsigned instr = ptrace(PTRACE_PEEKTEXT, child_pid, regs.rip, 0);

    //     // if 32-bit then regs.eip
    //     // if 64-bit then regs.rip
    //     procmsg("icounter = %u.  IP = 0x%08x.  instr = 0x%016x\n",
    //                 icounter, regs.rip, instr);

    //     /* Make the child execute another instruction */
    //     if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
    //         perror("ptrace");
    //         return;
    //     }

    //     /* Wait for child to stop on its next instruction */
    //     wait(&wait_status);
    // }

    procmsg("the child executed %u instructions\n", icounter);
}

