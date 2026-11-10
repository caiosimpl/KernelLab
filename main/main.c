#include <stdint.h>
#include "riscv/rv_utils.h"
#include "esp_rom_sys.h"
#include "pcb.h"
#include "tests.h"

volatile uint32_t tick_count = 0;
extern uint32_t meu_vector_table;

void tarefa_init_preempt(pcb_t *pcb, uint32_t *stack_mem, void (*func)(void));

static pcb_t tcb_main, tcb_a, tcb_b;
static uint32_t pstack_a[1024] __attribute__((aligned(16)));
static uint32_t pstack_b[1024] __attribute__((aligned(16)));

// bootstrap: o contexto do app_main vai pra ca na primeira troca
pcb_t *current_task = &tcb_main;

// chamada pelo trap.S: salva o sp atual, escolhe o proximo (A<->B), devolve o sp dele
uint32_t *pick_next_sp(uint32_t *sp_atual) {
    current_task->sp = sp_atual;
    if (current_task == &tcb_a) current_task = &tcb_b;
    else                        current_task = &tcb_a;
    return current_task->sp;
}

static void preempt_task_a(void) {
    int i = 0;
    while (1) {
        esp_rom_printf("[A] %d\n", i++);
        for (volatile int d = 0; d < 3000000; d++) {}
    }
}

static void preempt_task_b(void) {
    int i = 0;
    while (1) {
        esp_rom_printf("[B] %d\n", i++);
        for (volatile int d = 0; d < 3000000; d++) {}
    }
}

void app_main(void) {
    test_table();
    test_context_switch();

    tarefa_init_preempt(&tcb_a, pstack_a, preempt_task_a);
    tarefa_init_preempt(&tcb_b, pstack_b, preempt_task_b);

    rv_utils_set_mtvec((uint32_t)&meu_vector_table);
    while (1) {}
}
