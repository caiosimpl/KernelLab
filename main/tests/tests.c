#include <stdio.h>
#include "pcb.h"
#include "process_table.h"

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

