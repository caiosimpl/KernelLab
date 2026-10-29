#include "pcb.h"
#include "process_table.h"
#define QUEUE_SIZE 16
#define QUANTUM 1

static uint16_t queue [QUEUE_SIZE];

void run(process) { 
    void tarefa_init(pcb_t *process, uint32_t *stack_mem, void (*func)(void));
} 

pcb_t* setup(queue_position){
    uint16_t pid = queue[queue_position];
    pcb_t *process = table_get(pid);

    if(process == NULL || process->state != READY) {
        return NULL;
    };

    return process;
}

void main(){
    int32_t queue_pointer = -1;
    while(1) {
        queue_pointer = (queue_pointer +1) % QUEUE_SIZE;
         pcb_t* process = setup(queue_pointer);
         if(process != NULL) {
            run(process);
         }
    };
}