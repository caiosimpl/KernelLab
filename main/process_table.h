#ifndef PROCESS_TABLE_H
#define PROCESS_TABLE_H

#include "pcb.h"

void table_init(void);
pcb_t* table_create(const char *name, uint8_t priority);
pcb_t* table_get(uint16_t pid);
void table_remove(uint16_t pid);
void table_list(void);

#endif
