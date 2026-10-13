#include "pcb.h"

#define STACK_WORDS 1024   // tamanho da pilha em palavras de 32 bits

// prepara um processo novo pra poder ser escalonado pela primeira vez
void tarefa_init(pcb_t *pcb, uint32_t *stack_mem, void (*func)(void)) {
    
	uint32_t *sp = stack_mem + STACK_WORDS;        // topo da pilha (cresce pra baixo)
    sp = (uint32_t *)((uint32_t)sp & ~0xF);        // alinha a 16 bytes
    sp -= 16;                                      // abre 64 bytes (mesmo do switch.S)
    
	for (int i = 0; i < 16; i++) sp[i] = 0;        // zera o frame
    
	sp[0] = (uint32_t)func;                        // offset 0 = ra -> funcao da tarefa
    
	pcb->sp = sp;
    pcb->stack_base = stack_mem;
}
