#include "syscall.h"

// Aqui vai ficar definido como cada syscall se comunica com o kernel
// Sempre escrevemos um numero em a7 que vai identificar para o Kernel o que o programa quer
// Via de regra o registradore (a1, a2... a6) vao conter os argumentos e a saida em a0

int getpid(void) {
    register long a7 asm("a7") = SYS_GETPID;  // numero no a7
    register long a0 asm("a0");               // vai receber o retorno
    __asm__ volatile ("ecall" // chamada pro Kernel
        : "=r"(a0)     // saida: a0 = retorno do kernel
        : "r"(a7)      // entrada: a7 = numero
        : "memory");
    return (int)a0;
}
