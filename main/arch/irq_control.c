#include "irq_control.h"
#include "esp_cpu.h"

uint32_t kernel_irq_isolate(void)
{
    // Captura a mascara original.
    uint32_t original_mask =
        esp_cpu_intr_get_enabled_mask();

    // Desabilita todas as linhas habilitadas,
    // exceto a linha do GPTimer.
    esp_cpu_intr_disable(
        original_mask & ~KERNEL_TIMER_MASK
    );

    return original_mask;
}

void kernel_irq_restore(uint32_t original_mask)
{
    uint32_t current_mask =
        esp_cpu_intr_get_enabled_mask();

    // Desabilita linhas que nao estavam
    // habilitadas originalmente.
    esp_cpu_intr_disable(
        current_mask & ~original_mask
    );

    // Recupera as linhas originais.
    esp_cpu_intr_enable(
        original_mask & ~current_mask
    );
}
