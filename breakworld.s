    # https://cs.lmu.edu/~ray/notes/gasexamples/

    # The _start symbol must be declared for the linker (ld)
    .global _start

    # from there is the code, confusingly called text
    .text

_start:
    mov    $1, %rax                 # system call 1 is write
    mov    $1, %rdx                 # file handle 1 is stdout
    mov    $message1, %rsi          # address of the string to output
    mov    $4, %rdx                # number of bytes
    syscall                         # invoke the operating system call
    

    mov    $1, %rax                 # system call 1 is write
    mov    $1, %rdx                 # file handle 1 is stdout
    mov    $message2, %rsi          # address of the string to output
    mov    $6, %rdx                # number of bytes
    syscall                         # invoke the operating system call
    
    # Execute sys_exit
    mov    $60, %rax        # system call 60 is exit
    xor    %rdi, %rdi       # we want return code 0, xor X, X is always 0
    syscall                 # invoke operating system call to exit

    .data

message1:
    .ascii "Bye\n" # our message

message2:
    .ascii "world\n"
