#include "pcb.h"
#include "process_table.h"
#include <stdint.h>
#include <stddef.h>

#define QUEUE_SIZE 16
#define QUANTUM 1
#define QUEUE_VAZIA 0xFFFF // Necessario definir esse valor pq como as posicoes começam em 0, pode ter um processo de pid 0 que se repetiria 16 vezes

static uint16_t queue[QUEUE_SIZE];
static int32_t queue_pointer = -1; // Precisa ser declarado aqui para sobreviver entre chamadas
static pcb_t *atual = NULL;

// Inicializa a fila com valores QUEUE_VAZIA
static void fila_init(void) {
    for(int i = 0; i < QUEUE_SIZE; i++) {
        queue[i] = QUEUE_VAZIA;
    }
}

// Encontra o primeiro valor livre da queue e adiciona o PID, retornando 0. Caso esteja cheia retorna -1
static int fila_insere(uint16_t pid) {
    for(int i = 0; i < QUEUE_SIZE; i++) {
        if (queue[i] == QUEUE_VAZIA) {
            queue[i] = pid;
            return 0;
        }
    }
    return -1;
}

static pcb_t *setup(int32_t queue_position) {
    uint16_t pid = queue[queue_position];
    if (pid == QUEUE_VAZIA) return NULL;
    pcb_t *process = table_get(pid);

    if(process == NULL || process->state != READY) {
        return NULL;
    }

    return process;
}

void app_main(void) {
    fila_init();
    while(1) {
        queue_pointer = (queue_pointer +1) % QUEUE_SIZE;
        pcb_t* process = setup(queue_pointer);
    }
}