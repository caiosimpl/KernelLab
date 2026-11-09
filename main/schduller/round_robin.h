#ifndef ROUND_ROBIN_H
#define ROUND_ROBIN_H

#include "pcb.h"

#define RR_QUEUE_SIZE 16

void rr_init(void);
int rr_add(uint16_t pid);
void rr_remove(uint16_t pid);
pcb_t *rr_schedule(void);

#endif
