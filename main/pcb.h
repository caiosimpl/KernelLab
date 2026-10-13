#ifndef PCB_H
#define PCB_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
	FREE,
	READY,
	RUNNING,
	BLOCKED,
	DEAD
} state_t;


typedef struct {
    uint32_t *sp;          // stack pointer salvo da tarefa
    uint32_t *stack_base;  // base da pilha alocada (pra referencia)
	uint16_t pid;          // pid 
	char name[16];		   // nome
	state_t state;		   // estado
	uint8_t priority;	   // prioridade
	uint32_t created_at;   // timestamp de criacao
	

} pcb_t;


_Static_assert(offsetof(pcb_t, sp) == 0, "sp precisa ser o primeiro campo em pcb_t");

#endif

