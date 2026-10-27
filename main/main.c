#include <stdint.h>
#include "riscv/rv_utils.h"
#include "esp_rom_sys.h"

volatile uint32_t tick_count = 0;      // o interceptador incrementa isso
extern uint32_t meu_vector_table;      // a tabela do trap.S

void app_main(void) {
    // troca o mtvec pra NOSSA tabela
    rv_utils_set_mtvec((uint32_t)&meu_vector_table);

    while (1) {
        esp_rom_printf("tick_count = %u\n", tick_count);
        for (volatile int i = 0; i < 20000000; i++) { }   // espera na marra
    }
}
