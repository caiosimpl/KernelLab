#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "esp_err.h"
#include "esp_rom_sys.h"
#include "esp_intr_alloc.h"
#include "riscv/rv_utils.h"
#include "pcb.h"
#include "process_table.h"
#include "round_robin.h"
#include "kernel_timer.h"
#include "irq_control.h"

// definido em tarefa.c (nao tem header proprio)
void tarefa_init_preempt(pcb_t *pcb, uint32_t *stack_mem, void (*func)(void));

// nossa tabela de vetores, definida em arch/trap.S
extern void meu_vector_table(void);

// ---- estado do kernel ----
static pcb_t *tcb_a = NULL;
static pcb_t *tcb_b = NULL;

// PCB inicial: guarda o contexto do app_main na 1a troca.
// NAO entra na fila do RR -> e abandonado depois do 1o tick.
static pcb_t host_task;

// tarefa que esta na CPU agora
static pcb_t *current_task = NULL;

// chamada pelo trap.S (meu_intercept): salva o sp atual e devolve
// o sp da proxima tarefa escolhida pelo escalonador.
uint32_t *pick_next_sp(uint32_t *sp_atual)
{
    if (current_task == NULL) {
        return sp_atual;              // seguranca: ainda nao bootstrapou
    }

    current_task->sp = sp_atual;      // salva o contexto da que sai

    pcb_t *next = rr_schedule();      // pergunta ao round-robin quem entra
    if (next == NULL) {
        return sp_atual;              // ninguem pronto: segue na mesma
    }

    current_task = next;
    return next->sp;                  // devolve o contexto da que entra
}

// ---- tarefas de teste ----
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
    // 1. Tabela + escalonador
    table_init();
    rr_init();

    // 2. Cria as tarefas e monta os frames iniciais
    tcb_a = table_create("tarefa A", 1);
    tcb_b = table_create("tarefa B", 1);
    if (tcb_a == NULL || tcb_b == NULL) {
        esp_rom_printf("Erro ao criar processos\n");
        return;
    }
    tarefa_init_preempt(tcb_a, tcb_a->stack_base, preempt_task_a);
    tarefa_init_preempt(tcb_b, tcb_b->stack_base, preempt_task_b);

    // 3. Enfileira no round-robin
    if (rr_add(tcb_a->pid) != 0 || rr_add(tcb_b->pid) != 0) {
        esp_rom_printf("Erro ao adicionar processos\n");
        return;
    }

    // 4. BOOTSTRAP: o contexto atual (app_main) vira a tarefa hospedeira.
    current_task = &host_task;

    // 5. Configura o gptimer (aloca a linha de IRQ e habilita)
    ESP_ERROR_CHECK(kernel_timer_init());
    esp_intr_dump(stdout);   // CHECK unico: em qual linha o gptimer caiu?

    // 6. Instala a NOSSA tabela de vetores PERMANENTEMENTE.
    rv_utils_set_mtvec((uint32_t)(uintptr_t)&meu_vector_table);

    // 7. Isola: so a linha 5 (gptimer) vive. Silencia o tick do FreeRTOS.
    kernel_irq_isolate();

    // 8. Liga o timer: dispara a 100 Hz -> meu_intercept -> troca.
    ESP_ERROR_CHECK(kernel_timer_start());

    while (1) { }
}
