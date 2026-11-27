#ifndef SYSCALL_H
#define SYSCALL_H

// Contrato das syscalls do KernelLab
// Incluido pelos wrappers (lado do programa) e pelo dispatcher (lado do kernel).

enum {
    SYS_INVALID = 0,   // reservado: numero invalido / "nenhuma"
    SYS_GETPID  = 1,
    SYS_WRITE   = 2,
    SYS_EXIT    = 3,
    SYS_YIELD   = 4,

    SYS_COUNT          // SEMPRE por ultimo: vale a quantidade total
};

#endif
