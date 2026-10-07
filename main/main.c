#include <stdio.h>
#include "tcb.h"

extern void context_switch(tcb_t *atual, tcb_t *proxima);
void tarefa_init(tcb_t *tcb, uint32_t *stack_mem, void (*func)(void));

static tcb_t tcb_main, tcb_a, tcb_b;
static uint32_t stack_a[1024] __attribute__((aligned(16)));
static uint32_t stack_b[1024] __attribute__((aligned(16)));

void tarefa_A(void) {
    int cont = 0;
    while (cont < 100) {
        printf("  [A] %d\n", cont);
		cont++;
    }

	context_switch(&tcb_a, &tcb_b);
}

void tarefa_B(void) {
    int cont = 0;
    while (cont < 100) {
        printf("  [B] %d\n", cont);
		cont++;
    }

	context_switch(&tcb_b, &tcb_a);
}

void app_main(void) {
    printf("=== troca de contexto (versao final) ===\n");

    tarefa_init(&tcb_a, stack_a, tarefa_A);
    tarefa_init(&tcb_b, stack_b, tarefa_B);

    context_switch(&tcb_main, &tcb_a);
}

