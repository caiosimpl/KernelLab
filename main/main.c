#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include "riscv/rv_utils.h"
#include "esp_rom_sys.h"
#include "pcb.h"
#include "tests.h"
#include "process_table.h"
#include "schduller/round_robin.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "kernel_timer.h"
#include "esp_err.h"
#include <stdio.h>
#include "esp_intr_alloc.h"
#include "arch/irq_control.h"
#include "esp_cpu.h"
#include <stdbool.h>

void tarefa_init_preempt(pcb_t *pcb, uint32_t *stack_mem, void (*func)(void));
extern void context_switch(pcb_t *atual, pcb_t *proxima);
extern void tarefa_init(pcb_t *pcb, uint32_t *stack_mem, void (*func)(void));

extern volatile uint32_t kernel_irq_mcause;
extern volatile uint32_t kernel_irq_mtvec;
extern volatile uint32_t kernel_irq_captured;

static pcb_t *tcb_a = NULL;
static pcb_t *tcb_b = NULL;

static pcb_t *current_task = NULL;

// teste bootstrap {{{

static pcb_t probe_pcb;

static uint32_t probe_stack[1024]
    __attribute__((aligned(16)));

volatile uint32_t probe_marker = 0;
volatile uint32_t probe_t2_seen = 0;

extern void kernel_probe_run(uint32_t *frame);
extern void kernel_probe_task(void);

volatile uint32_t probe_mtvec_seen = 0;
extern void meu_vector_table(void);

static void teste_primeiro_mret(void)
{
    esp_rom_printf("\n=== TESTE PRIMEIRO MRET ===\n");

    probe_marker = 0;

    tarefa_init_preempt(
        &probe_pcb,
        probe_stack,
        kernel_probe_task
    );

    if (probe_pcb.sp == NULL) {
        esp_rom_printf("ERRO: frame invalido\n");
        return;
    }

    if (((uintptr_t)probe_pcb.sp & 0xF) != 0) {
        esp_rom_printf("ERRO: SP desalinhado\n");
        return;
    }

    // MPP=M, MPIE=0: retorna com
    // interrupcoes globais desabilitadas.
    probe_pcb.sp[32] = UINT32_C(0x1800);

    esp_rom_printf(
        "SP inicial: %p\n",
        (void *)probe_pcb.sp
    );

    esp_rom_printf("Entrando no contexto...\n");

    probe_t2_seen = 0;
    probe_pcb.sp[6] = UINT32_C(0x12345678);

    uint32_t mtvec_before;
   uint32_t mtvec_after;

   probe_mtvec_seen = 0;

   __asm__ volatile (
       "csrr %0, mtvec"
       : "=r"(mtvec_before)
   );

    // Durante esta chamada nao usamos APIs do FreeRTOS.
    kernel_probe_run(probe_pcb.sp);

    __asm__ volatile (
       "csrr %0, mtvec"
       : "=r"(mtvec_after)
   );

   // Endereço real da tabela de vetores do KernelLab
   uintptr_t vector_addr = (uintptr_t)&meu_vector_table;

   // Valor esperado de mtvec no modo vetorizado
   uint32_t expected_mtvec =
       (uint32_t)vector_addr | 1u;

   esp_rom_printf("\n=== TESTE MTVEC ===\n");

   esp_rom_printf(
       "Original:   0x%08lx\n",
       (unsigned long)mtvec_before
   );

   esp_rom_printf(
       "Esperado:   0x%08lx\n",
       (unsigned long)expected_mtvec
   );

   esp_rom_printf(
       "KernelLab:  0x%08lx\n",
       (unsigned long)probe_mtvec_seen
   );

   esp_rom_printf(
       "Restaurado: 0x%08lx\n",
       (unsigned long)mtvec_after
   );

   bool mtvec_ok =
       probe_mtvec_seen == expected_mtvec &&
       mtvec_after == mtvec_before &&
       probe_mtvec_seen != mtvec_before;

   esp_rom_printf(
       "Troca de mtvec: %s\n",
       mtvec_ok ? "PASSOU" : "FALHOU"
   );

    // So chegamos aqui se o contexto hospedeiro foi restaurado.
    esp_rom_printf("Retornou ao hospedeiro\n");


    bool bootstrap_ok =
        probe_marker == UINT32_C(0x4B4C4142);

    bool t2_ok =
        probe_t2_seen == UINT32_C(0x12345678);

    esp_rom_printf(
       "Bootstrap: %s\n",
        bootstrap_ok ? "PASSOU" : "FALHOU"
    );

    esp_rom_printf(
       "Restauracao de t2: %s\n",
       t2_ok ? "PASSOU" : "FALHOU"
    );

    esp_rom_printf(
       "t2 observado: 0x%08lx\n",
       (unsigned long)probe_t2_seen);

   esp_rom_printf(
       "Endereco da tabela: 0x%08lx\n",
       (unsigned long)vector_addr
   );

   if ((vector_addr & 0xFFu) != 0) {
       esp_rom_printf("ERRO: tabela desalinhada!\n");
       return;
   }
}

// }}}

// chamada pelo trap.S: salva o sp atual, escolhe o proximo (A<->B), devolve o sp dele
uint32_t *pick_next_sp(uint32_t *sp_atual)
{
    // O KernelLab ainda nao iniciou uma tarefa propria.
    // Preserva o contexto recebido.
    if (current_task == NULL) {
        return sp_atual;
    }

    // Salva o contexto da tarefa atual no PCB.
    current_task->sp = sp_atual;

    // Solicita ao Round-Robin a proxima tarefa.
    pcb_t *next = rr_schedule();

    // Tratamento provisório: nenhuma tarefa selecionada.
    if (next == NULL) {
        return sp_atual;
    }

    // Atualiza o processo corrente.
    current_task = next;

    // Devolve o contexto da tarefa escolhida.
    return next->sp;
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

static void teste_irq_isolation(void)
{
    uint32_t saved_mstatus;

    // Salva mstatus e desabilita MIE.
    __asm__ volatile (
        "csrrci %0, mstatus, 8"
        : "=r"(saved_mstatus)
        :
        : "memory"
    );

    uint32_t original = kernel_irq_isolate();

    uint32_t isolated =
        esp_cpu_intr_get_enabled_mask();

    kernel_irq_restore(original);

    uint32_t restored =
        esp_cpu_intr_get_enabled_mask();

    // Recupera mstatus original.
    __asm__ volatile (
        "csrw mstatus, %0"
        :
        : "r"(saved_mstatus)
        : "memory"
    );

    // Imprime somente apos restaurar
    // o ambiente original.
    esp_rom_printf("\n=== IRQ ISOLATION ===\n");

    esp_rom_printf(
        "Original: 0x%08lx\n",
        (unsigned long)original
    );

    esp_rom_printf(
        "Isolada:  0x%08lx\n",
        (unsigned long)isolated
    );

    esp_rom_printf(
        "Restaurada: 0x%08lx\n",
        (unsigned long)restored
    );

    bool ok =
        isolated == KERNEL_TIMER_MASK &&
        restored == original;

    esp_rom_printf(
        "Isolamento: %s\n",
        ok ? "PASSOU" : "FALHOU"
    );
}

void app_main(void) {

   table_init();  //inicializa a tabela de processos
   rr_init();  //inicializa a fila do RR

   //cria os PCBs
   tcb_a = table_create("tarefa A", 1);
   tcb_b =table_create("tarefa B", 1);

   if (tcb_a == NULL || tcb_b == NULL) {
        esp_rom_printf("Erro ao criar processos\n");
        return;
    }
   //contextos iniciais sendo preparados
    tarefa_init_preempt(tcb_a, tcb_a->stack_base, preempt_task_a);
    tarefa_init_preempt(tcb_b, tcb_b->stack_base, preempt_task_b);

    //adicionando processos ao escalonador
    if (rr_add(tcb_a->pid) != 0 ||
        rr_add(tcb_b->pid) != 0) {
        esp_rom_printf("Erro ao adicionar processos\n");
        return;
    }

    esp_rom_printf("\n=== PASSO 4: TIMER ===\n");

    ESP_ERROR_CHECK(kernel_timer_init());
    ESP_ERROR_CHECK(kernel_timer_start());

    // observando CSRs {{{
    vTaskDelay(pdMS_TO_TICKS(100));

   if (kernel_irq_captured) {
       uint32_t mcause = kernel_irq_mcause;
       uint32_t mtvec = kernel_irq_mtvec;

       uint32_t is_interrupt = mcause >> 31;
       uint32_t cause_number = mcause & 0x7FFFFFFFu;

       esp_rom_printf("\n=== GPTIMER IRQ ===\n");

       esp_rom_printf(
           "mcause: 0x%08lx\n",
           (unsigned long)mcause
       );

       esp_rom_printf(
           "Interrupcao: %lu\n",
           (unsigned long)is_interrupt
       );

       esp_rom_printf(
           "Numero da causa: %lu\n",
           (unsigned long)cause_number
       );

       esp_rom_printf(
           "mtvec: 0x%08lx\n",
           (unsigned long)mtvec
       );

       esp_rom_printf(
           "Modo mtvec: %lu\n",
           (unsigned long)(mtvec & 0x3u)
       );
   }

   printf("\n=== INTERRUPCOES ALOCADAS ===\n");

   esp_intr_dump(stdout);
    // }}}
   teste_irq_isolation();

    esp_rom_printf("Timer iniciado em 100 Hz\n");

    uint32_t anterior = tick_count;

    tarefa_init_preempt(
    tcb_a, tcb_a->stack_base, preempt_task_a
    );

    tarefa_init_preempt(
        tcb_b, tcb_b->stack_base, preempt_task_b
    );

    uint32_t gp_atual;

   __asm__ volatile(
       "mv %0, gp"
       : "=r"(gp_atual)
   );

   uint32_t gp_a = tcb_a->sp[3];
   uint32_t gp_b = tcb_b->sp[3];

   esp_rom_printf(
       "GP hospedeiro: 0x%08lx\n"
       "GP tarefa A:   0x%08lx\n"
       "GP tarefa B:   0x%08lx\n",
       (unsigned long)gp_atual,
       (unsigned long)gp_a,
       (unsigned long)gp_b
   );

   esp_rom_printf(
       "Inicializacao de GP: %s\n",
       (gp_a == gp_atual && gp_b == gp_atual)
           ? "PASSOU" : "FALHOU"
   );

   teste_primeiro_mret();
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        uint32_t atual = tick_count;
        uint32_t diferenca = atual - anterior;

        anterior = atual;

        esp_rom_printf(
            "tick_count=%lu | ticks no intervalo=%lu\n",
            (unsigned long)atual,
            (unsigned long)diferenca
        );
    }

}
