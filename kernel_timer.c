#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

#include <stdint.h>
#include "esp_err.h"

extern volatile uint32_t tick_count;

esp_err_t kernel_timer_init(void);
esp_err_t kernel_timer_start(void);

#endif

