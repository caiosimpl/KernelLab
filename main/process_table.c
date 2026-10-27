#include <stdio.h>
#include <string.h>
#include "pcb.h"
#include "process_table.h"

#define MAX_PROCESSES 16

static pcb_t table[MAX_PROCESSES];
static uint16_t next_pid = 1;

void table_init(void) {
	for(int i = 0; i < MAX_PROCESSES; i++) {
		table[i].state = FREE;
	}
} 


// O created_at deve ser setado pelo Kernel
pcb_t* table_create(const char *name, uint8_t priority) {
	for(int i = 0; i < MAX_PROCESSES; i++) {
		if(table[i].state == FREE) {
			table[i].pid = next_pid++;
			table[i].state = READY;
			table[i].priority = priority;
			strncpy(table[i].name, name, 15);

			table[i].name[15] = '\0'; // saber onde a string acaba

			return &table[i];

		}
	}

	return NULL;
}


pcb_t* table_get(uint16_t pid) {
	for(int i = 0; i < MAX_PROCESSES; i++) {
		if(table[i].state != FREE && table[i].pid == pid) {
			return &table[i];
		}
	}

	return NULL;
}


void table_remove(uint16_t pid) {
    pcb_t *p = table_get(pid);
    if (p != NULL) {
        p->state = FREE;
    }
}


void table_list(void) {
	for(int i = 0; i < MAX_PROCESSES; i++) {
		if(table[i].state != FREE) {
			printf("pid=%d  name=%s  state=%d  prio=%d\n",
					table[i].pid,
					table[i].name,
					table[i].state,
					table[i].priority);
		}
	}
}

