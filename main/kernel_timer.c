#include <stdint.h>
#include <stdbool.h>

#include "driver/gptimer.h"
#include "esp_err.h"
#include "esp_attr.h"
#include "kernel_timer.h"

#define TIMER_RESOLUTION_HZ 1000000
#define KERNEL_TICK_HZ      100

#define ALARM_COUNT \
    (TIMER_RESOLUTION_HZ / KERNEL_TICK_HZ)

volatile uint32_t tick_count = 0;

volatile uint32_t kernel_irq_mcause = 0;
volatile uint32_t kernel_irq_mtvec = 0;
volatile uint32_t kernel_irq_captured = 0;

static gptimer_handle_t kernel_timer = NULL;

/*
 * Executada quando o temporizador gera
 * uma interrupcao.
 *
 * Ainda nao troca o contexto de tarefas.
 */
static bool IRAM_ATTR kernel_timer_isr(
    gptimer_handle_t timer,
    const gptimer_alarm_event_data_t *edata,
    void *user_ctx){

    (void)timer;
    (void)edata;
    (void)user_ctx;

    if (kernel_irq_captured == 0) {
        uint32_t mcause;
        uint32_t mtvec;

        __asm__ volatile (
            "csrr %0, mcause"
            : "=r"(mcause)
        );

        __asm__ volatile (
            "csrr %0, mtvec"
            : "=r"(mtvec)
        );

        kernel_irq_mcause = mcause;
        kernel_irq_mtvec = mtvec;
        kernel_irq_captured = 1;
    }

    tick_count++;

    return false;
}

esp_err_t kernel_timer_init(void)
{
    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION_HZ,
    };

    esp_err_t err = gptimer_new_timer(
        &config,
        &kernel_timer
    );

    if (err != ESP_OK) {
        return err;
    }

    gptimer_event_callbacks_t callbacks = {
        .on_alarm = kernel_timer_isr,
    };

    err = gptimer_register_event_callbacks(
        kernel_timer,
        &callbacks,
        NULL
    );

    if (err != ESP_OK) {
        return err;
    }

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = ALARM_COUNT,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };

    err = gptimer_set_alarm_action(
        kernel_timer,
        &alarm_config
    );

    if (err != ESP_OK) {
        return err;
    }

    return gptimer_enable(kernel_timer);
}

esp_err_t kernel_timer_start(void)
{
    if (kernel_timer == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    return gptimer_start(kernel_timer);
}

