#include "pcb.h"
#include "process_table.h"
#define QUEUE_SIZE 16
#define QUANTUM 1

static uint16_t queue[QUEUE_SIZE];
static int32_t queue_pointer = -1; // Precisa ser declarado aqui para sobreviver entre chamadas
static pcb_t *atual = NULL;



static void run(pcb_t *process) { 
    void tarefa_init(pcb_t *process, uint32_t *stack_mem, void (*func)(void));
} 

static pcb_t *setup(uint16_t queue_position) {
    uint16_t pid = queue[queue_position];
    pcb_t *process = table_get(pid);

    if(process == NULL || process->state != READY) {
        return NULL;
    }

    return process;
}

void app_main() {
    while(1) {
        queue_pointer = (queue_pointer +1) % QUEUE_SIZE;
         pcb_t* process = setup(queue_pointer);
         if(process != NULL) {
            run(process);
         }
    }
}