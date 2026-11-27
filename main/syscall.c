#include "syscall.h"
#include "pcb.h"

// o kernel precisa expor quem esta rodando agora.
// (o current_task vai virar isso quando a gente modularizar o kernel)
extern pcb_t *kernel_current(void);

// a implementacao do getpid: so le um dado do kernel e devolve
static long sys_getpid(void) {
    return kernel_current()->pid;
}

// o dispatcher: recebe o numero + args e executa oq tem que executar
long syscall_dispatch(uint32_t num,
                      uint32_t arg0, uint32_t arg1, uint32_t arg2,
                      uint32_t arg3, uint32_t arg4, uint32_t arg5) {
    switch (num) {
        case SYS_GETPID: return sys_getpid();
        // case SYS_WRITE:  return sys_write((const char*)arg0, arg1);
        // case SYS_EXIT:   ...
        default:         return -1;   // desconhecida
    }
}
