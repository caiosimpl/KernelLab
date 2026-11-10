#include "pcb.h"

// prepara um processo novo pra poder ser escalonado pela primeira vez
// esse cara eh oq a troca cooperativa espera
void tarefa_init(pcb_t *pcb, uint32_t *stack_mem, void (*func)(void)) {
    
	uint32_t *sp = stack_mem + STACK_WORDS;        // topo da pilha (cresce pra baixo)
    sp = (uint32_t *)((uint32_t)sp & ~0xF);        // alinha a 16 bytes
    sp -= 16;                                      // abre 64 bytes (mesmo do switch.S)
    
	for(int i = 0; i < 16; i++) sp[i] = 0;        // zera o frame
    
	sp[0] = (uint32_t)func;                        // offset 0 = ra -> funcao da tarefa
    
	pcb->sp = sp;
    pcb->stack_base = stack_mem;
}

// esse cara eh oq a troca preemptiva espera
void tarefa_init_preempt(pcb_t *pcb, uint32_t *stack_mem, void (*func)(void)) {
    uint32_t *top = stack_mem + STACK_WORDS;
    
	top = (uint32_t *)((uint32_t)top & ~0xF);     // alinha a 16
    uint8_t *frame = (uint8_t *)top - 144;        // abre o frame de 144 bytes
    for(int i = 0; i < 144; i++) frame[i] = 0;   // zera tudo

    *(uint32_t *)(frame + 124) = (uint32_t)func;  // offset 124 = mepc = func
    *(uint32_t *)(frame + 128) = 0x1880;          // offset 128 = mstatus

    pcb->sp = (uint32_t *)frame;
    pcb->stack_base = stack_mem;
}
