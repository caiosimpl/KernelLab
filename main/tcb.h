#ifndef TCB_H
#define TCB_H

#include <stdint.h>

typedef struct {
    uint32_t *sp;          // stack pointer salvo da tarefa
    uint32_t *stack_base;  // base da pilha alocada (pra refer├¬ncia)
} tcb_t;

#endif

