#ifndef IRQ_CONTROL_H
#define IRQ_CONTROL_H

#include <stdint.h>

#define KERNEL_TIMER_IRQ 5u
#define KERNEL_TIMER_MASK (1u << KERNEL_TIMER_IRQ)

uint32_t kernel_irq_isolate(void);
void kernel_irq_restore(uint32_t original_mask);

#endif
