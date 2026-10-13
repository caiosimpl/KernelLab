#include <stdio.h>
#include "pcb.h"

extern void context_switch(pcb_t *atual, pcb_t *proxima);
void tarefa_init(pcb_t *pcb, uint32_t *stack_mem, void (*func)(void));

static pcb_t pcb_main, pcb_a, pcb_b;
static uint32_t stack_a[1024] __attribute__((aligned(16)));
static uint32_t stack_b[1024] __attribute__((aligned(16)));

void tarefa_A(void) {
    int cont = 0;
    while (cont < 100) {
        printf("  [A] %d\n", cont);
		cont++;
    }

	context_switch(&pcb_a, &pcb_b);
}

void tarefa_B(void) {
    int cont = 0;
    while (cont < 100) {
        printf("  [B] %d\n", cont);
		cont++;
    }

	context_switch(&pcb_b, &pcb_a);
}

void app_main(void) {
    printf("=== troca de contexto (versao final) ===\n");

    tarefa_init(&pcb_a, stack_a, tarefa_A);
    tarefa_init(&pcb_b, stack_b, tarefa_B);

    context_switch(&pcb_main, &pcb_a);
}

