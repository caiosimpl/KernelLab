#include <stdio.h>
#include "pcb.h"
#include "process_table.h"
#include "schduller/round_robin.h"

static int _ok, _fail;


// Isso ta muito feio mas eh pq o define te obriga a deixar tudo na mesma linha
// o define funciona assim CHECK recebe uma condicao booleana para ser avaliada e uma msg, imprime OK ou FAIL conforme a condicao eh avaliada
// printa [OK] msg ou [FAIL] msg

#define CHECK(cond, msg) do { if (cond) { _ok++; printf("  [OK]   %s\n", msg); } else { _fail++; printf("  [FAIL] %s\n", msg); } } while (0)

static void _resumo(const char *name) {
	printf(">> %s: %d OK, %d FAIL\n\n", name, _ok, _fail);
	_ok = 0;
	_fail = 0;
}


// maquinário para troca de contexto

extern void context_switch(pcb_t *atual, pcb_t *proxima);
void tarefa_init(pcb_t *pcb, uint32_t *stack_mem, void (*func)(void));

static pcb_t cs_main, cs_a, cs_b;
static uint32_t stack_a[1024] __attribute__((aligned(16)));
static uint32_t stack_b[1024] __attribute__((aligned(16)));


static void task_a(void) {
    for(int i = 0; i < 5; i++) {
        printf("  [A] %d\n", i);
        context_switch(&cs_a, &cs_b); // cede a vez pra B
    }
    context_switch(&cs_a, &cs_main);// no fim, volta pro main
}


static void task_b(void) {
    for(int i = 0; i < 5; i++) {
        printf("  [B] %d\n", i);
        context_switch(&cs_b, &cs_a); // cede a vez pra A
    }
    context_switch(&cs_b, &cs_main);
}



void test_context_switch(void) {
    printf("\n==== test_context_switch ====\n");
    tarefa_init(&cs_a, stack_a, task_a);
    tarefa_init(&cs_b, stack_b, task_b);
    context_switch(&cs_main, &cs_a); //comeca em A
    printf(">> voltou pro main, troca de contexto OK\n\n");
}


// Cada tarefa executa tres vezes, cedendo a CPU a cada tick simulado.
#define RR_TEST_ROUNDS 3

static pcb_t rr_main;
static pcb_t *rr_current;
static pcb_t *rr_tasks[RR_QUEUE_SIZE];
static unsigned rr_runs[RR_QUEUE_SIZE];
static uint16_t rr_trace[RR_QUEUE_SIZE * RR_TEST_ROUNDS];
static unsigned rr_trace_size;
static unsigned rr_ticks;

// Simula a chamada do escalonador ao fim de um quantum. Nao e uma ISR real:
// o switch.S atual salva o contexto de uma chamada de funcao cooperativa.
static void rr_interrupt_simulated(void) {
    pcb_t *previous = rr_current;
    pcb_t *next = rr_schedule();

    rr_ticks++;
    printf("  [tick %u] %s -> %s\n", rr_ticks, previous->name,
           next != NULL ? next->name : "main");

    CHECK(previous->state != RUNNING, "tarefa anterior deixa o estado RUNNING");
    rr_current = next;

    if (next == NULL) {
        context_switch(previous, &rr_main);
    } else {
        CHECK(next->state == RUNNING, "proxima tarefa esta RUNNING");
        if (next != previous) {
            context_switch(previous, next);
        }
    }
}

static void rr_task(void) {
    pcb_t *self = rr_current;
    unsigned index = 0;

    while (index < RR_QUEUE_SIZE && rr_tasks[index] != self) {
        index++;
    }
    CHECK(index < RR_QUEUE_SIZE, "tarefa pertence ao conjunto do teste");
    if (index == RR_QUEUE_SIZE) {
        context_switch(self, &rr_main);
        return;
    }

    for (int i = 0; i < RR_TEST_ROUNDS; i++) {
        printf("  [%s] %d\n", self->name, i);
        rr_runs[index]++;
        if (rr_trace_size < RR_QUEUE_SIZE * RR_TEST_ROUNDS) {
            rr_trace[rr_trace_size++] = self->pid;
        } else {
            CHECK(0, "registro de execucoes nao deve exceder o limite");
        }
        rr_interrupt_simulated();
        CHECK(rr_current == self && self->state == RUNNING,
              "tarefa retoma seu contexto apos o tick");
    }

    self->state = DEAD;
    rr_remove(self->pid);
    rr_interrupt_simulated(); // A ultima tarefa devolve o controle ao main.
}

static int rr_prepare_tasks(unsigned count) {
    table_init();
    rr_init();
    rr_current = NULL;
    rr_trace_size = 0;
    rr_ticks = 0;

    for (unsigned i = 0; i < RR_QUEUE_SIZE; i++) {
        rr_tasks[i] = NULL;
        rr_runs[i] = 0;
    }

    CHECK(rr_schedule() == NULL, "fila vazia nao seleciona tarefa");
    for (unsigned i = 0; i < count; i++) {
        char name[16];
        snprintf(name, sizeof(name), "RR-%02u", i + 1);
        rr_tasks[i] = table_create(name, 1);
        CHECK(rr_tasks[i] != NULL, "cria tarefa na tabela");
        if (rr_tasks[i] == NULL) {
            return 0;
        }

        tarefa_init(rr_tasks[i], rr_tasks[i]->stack_base, rr_task);
        int added = rr_add(rr_tasks[i]->pid);
        CHECK(added == 0, "adiciona tarefa READY na fila");
        if (added != 0) {
            return 0;
        }
    }
    return 1;
}

static void rr_run_tasks(unsigned count) {
    rr_current = rr_schedule();
    CHECK(rr_current == rr_tasks[0], "primeira tarefa inicia a rodada");
    if (rr_current == NULL) {
        return;
    }
    context_switch(&rr_main, rr_current);

    CHECK(rr_current == NULL, "ultima tarefa devolve o controle ao main");
    CHECK(rr_trace_size == count * RR_TEST_ROUNDS,
          "todas as rodadas foram executadas");
    CHECK(rr_ticks == count * (RR_TEST_ROUNDS + 1),
          "cada tarefa gera ticks de execucao e de encerramento");
    for (unsigned i = 0; i < count; i++) {
        CHECK(rr_runs[i] == RR_TEST_ROUNDS, "tarefa executou tres vezes");
        CHECK(rr_tasks[i]->state == DEAD, "tarefa encerrada esta DEAD");
    }
    for (unsigned i = 0; i < rr_trace_size; i++) {
        CHECK(rr_trace[i] == rr_tasks[i % count]->pid,
              "ordem circular preservada entre os ticks");
    }
    CHECK(rr_schedule() == NULL, "fila fica vazia apos todas as tarefas encerrarem");
}

void test_round_robin(void) {
    printf("\n==== test_round_robin ====\n");
    if (rr_prepare_tasks(3)) {
        CHECK(rr_add(rr_tasks[0]->pid) == -1, "rejeita tarefa duplicada");
        rr_run_tasks(3);
    }
    _resumo("test_round_robin");
}

void test_round_robin_queue_full(void) {
    printf("\n==== test_round_robin_queue_full ====\n");
    if (!rr_prepare_tasks(RR_QUEUE_SIZE)) {
        _resumo("test_round_robin_queue_full");
        return;
    }

    CHECK(table_create("excedente", 1) == NULL,
          "tabela cheia rejeita a 17a tarefa");

    // A tabela e a fila possuem 16 vagas. Substitui um PCB, deixando o PID
    // antigo na fila para tentar inserir um PID valido com a fila cheia.
    uint16_t old_pid = rr_tasks[RR_QUEUE_SIZE - 1]->pid;
    table_remove(old_pid);
    pcb_t *extra = table_create("RR-extra", 1);
    CHECK(extra != NULL, "cria tarefa substituta com novo PID");
    if (extra == NULL) {
        _resumo("test_round_robin_queue_full");
        return;
    }
    CHECK(rr_add(extra->pid) == -1, "fila cheia rejeita novo PID READY");

    rr_remove(old_pid);
    tarefa_init(extra, extra->stack_base, rr_task);
    rr_tasks[RR_QUEUE_SIZE - 1] = extra;
    int added = rr_add(extra->pid);
    CHECK(added == 0, "reutiliza vaga liberada na fila");
    if (added == 0) {
        rr_run_tasks(RR_QUEUE_SIZE);
    }
    _resumo("test_round_robin_queue_full");
}



// teste da tabela
void test_table(void) {
	printf("\n==== test_table ====\n");

	table_init();

	pcb_t *a = table_create("A", 3);

	CHECK(a != NULL, "create deve retornar um ponteiro valido");
	CHECK(a->pid == 1, "pid de A deve ser igual 1");

	pcb_t *b = table_create("B", 2);

	CHECK(b != NULL,  "create de deve retornar um ponteiro valido");
	CHECK(b-> pid == 2,  "pid de de B dever igual a 2");

	CHECK(table_get(2) == b,  "processo de pid 2 deve ser B");
	CHECK(table_get(1) == a,   "processo de pid 1 deve ser A");
	CHECK(table_get(3) == NULL, "processo de pid 3 deve retornar NULL");


	table_remove(2);
	CHECK(table_get(2) == NULL, "removido B, 2 deve retornar NULL");

	pcb_t *c = table_create("C", 2);
	CHECK(c->pid == 3,           "vaga reusada mas pid novo (3, nao 2)");

	_resumo("test_table");


}

