#include "tcb.h"

#define STACK_WORDS 1024   // tamanho da pilha em palavras de 32 bits

// prepara uma tarefa nova pra poder ser escalonada pela primeira vez
void tarefa_init(tcb_t *tcb, uint32_t *stack_mem, void (*func)(void)) {
    // aponta pro topo da pilha (cresce pra baixo)
    uint32_t *sp = stack_mem + STACK_WORDS;

    // ALINHA a 16 bytes
    sp = (uint32_t *)((uint32_t)sp & ~0xF);

    // abre espa├ºo pros 16 slots (64 bytes ÔÇö mesmo do switch.S, mant├®m alinhado)
    sp -= 16;

    // zera tudo
    for (int i = 0; i < 16; i++) sp[i] = 0;

    // offset 0 = ra ÔåÆ o endere├ºo da fun├º├úo da tarefa
    // quando o context_switch fizer 'ret', pula pra c├í
    sp[0] = (uint32_t)func;

    // salva o sp preparado no TCB
    tcb->sp = sp;
    tcb->stack_base = stack_mem;
}

