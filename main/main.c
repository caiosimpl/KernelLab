#include "context.h"
#include <stdio.h>

#define STACK_SIZE 2048

static uint8_t stack_tarefa_a[STACK_SIZE] __attribute__((aligned(16)));
static uint8_t stack_tarefa_b[STACK_SIZE] __attribute__((aligned(16)));

struct context ctx_main; 
struct context ctx_a;
struct context ctx_b;

void tarefa_a(void) {
    int contador = 0;
    while (1) {
        printf("Tarefa A: %d\n", contador++);
        for (volatile int i = 0; i < 500000; i++); 
        context_switch(&ctx_a, &ctx_b);
    }
}

void tarefa_b(void) {
    int contador = 0;
    while (1) {
        printf("Tarefa B: %d\n", contador++);
        for (volatile int i = 0; i < 500000; i++);
        context_switch(&ctx_b, &ctx_a);
    }
}

void app_main(void) {
    // Monta o contexto inicial de cada tarefa, tudo na mão
    ctx_a.ra = (uint32_t) tarefa_a;
    ctx_a.sp = (uint32_t)(stack_tarefa_a + STACK_SIZE); //topo da pilha

    ctx_b.ra = (uint32_t) tarefa_b;
    ctx_b.sp = (uint32_t)(stack_tarefa_b + STACK_SIZE);

    context_switch(&ctx_main, &ctx_a);
}
