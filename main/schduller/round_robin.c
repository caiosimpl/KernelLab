#include "pcb.h"
#include "process_table.h"

#include <stdint.h>
#include <stddef.h>

#define QUEUE_SIZE 16
#define QUANTUM 1
#define QUEUE_VAZIA 0xFFFF // Necessario definir esse valor pq como as posicoes começam em 0, pode ter um processo de pid 0 que se repetiria 16 vezes

static uint16_t queue[QUEUE_SIZE];
static int32_t queue_pointer = QUEUE_SIZE -1; // Precisa ser declarado aqui para sobreviver entre chamadas


// Inicializa a fila com valores QUEUE_VAZIA
void rr_init(void) {
    for(int i = 0; i < QUEUE_SIZE; i++) {
        queue[i] = QUEUE_VAZIA;
    }

    queue_pointer = QUEUE_SIZE - 1;
}

// Encontra o primeiro valor livre da queue e adiciona o PID, retornando 0. Caso esteja cheia retorna -1
int rr_add(uint16_t pid) {

    pcb_t *process = table_get(pid);

    if (process == NULL || process->state != READY) {
        return -1;
    }
    // Evita inserir o msm processo duas vzs
    for (int i = 0; i < QUEUE_SIZE; i++) {
        if (queue[i] == pid) {
            return -1;
        }
    }
    // Procura uma posicao livre
    for (int i = 0; i < QUEUE_SIZE; i++) {
        if (queue[i] == QUEUE_VAZIA) {
            queue[i] = pid;
            return 0;
        }
    }

    return -1;
}

void rr_remove(uint16_t pid) {

    for (int i = 0; i < QUEUE_SIZE; i++) {
        if (queue[i] == pid) {
            queue[i] = QUEUE_VAZIA;
            return;
        }
    }

}

static pcb_t *rr_find_next(void) {

    for (int i = 1; i <= QUEUE_SIZE; i++) {

        int32_t position = (queue_pointer + i) % QUEUE_SIZE;

        uint16_t pid = queue[position];

        if (pid == QUEUE_VAZIA) {
            continue;
        }

        pcb_t *process = table_get(pid);

        if (process == NULL) {
            queue[position] = QUEUE_VAZIA;
            continue;
        }

        if (process->state == READY) {
            queue_pointer = position;
            return process;
        }
    }

    return NULL;
}

pcb_t *rr_schedule(void) {

    uint16_t current_pid = queue[queue_pointer];

    if (current_pid != QUEUE_VAZIA) {

        pcb_t *current = table_get(current_pid);

        if (current != NULL && current->state == RUNNING) {
            current->state = READY;
        }
    }

    pcb_t *next = rr_find_next();

    if (next == NULL) {
        return NULL;
    }

    next->state = RUNNING;

    return next;
}