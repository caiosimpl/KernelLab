/* arch/trap.S — tabela de vetores propria do KernelLab */
    .global _interrupt_handler
    .global _panic_handler

    /* ===== a NOSSA tabela de vetores ===== */
    .section .iram1, "ax"
    .balign 0x100                 /* OBRIGATORIO: alinhar em 256 */
    .option push
    .option norvc                 /* OBRIGATORIO: cada 'j' = 4 bytes */
    .global meu_vector_table
    .type meu_vector_table, @function
meu_vector_table:
    j _panic_handler        /* 0: excecao */
    j _interrupt_handler    /* 1 */
    j _interrupt_handler    /* 2 */
    j _interrupt_handler    /* 3 */
    j meu_intercept   /* 4: TICK  <-- PASSO 2: trocar para  j meu_intercept */
    j _interrupt_handler    /* 5 */
    j _interrupt_handler    /* 6 */
    j _interrupt_handler    /* 7 */
    j _interrupt_handler    /* 8 */
    j _interrupt_handler    /* 9 */
    j _interrupt_handler    /* 10 */
    j _interrupt_handler    /* 11 */
    j _interrupt_handler    /* 12 */
    j _interrupt_handler    /* 13 */
    j _interrupt_handler    /* 14 */
    j _interrupt_handler    /* 15 */
    j _interrupt_handler    /* 16 */
    j _interrupt_handler    /* 17 */
    j _interrupt_handler    /* 18 */
    j _interrupt_handler    /* 19 */
    j _interrupt_handler    /* 20 */
    j _interrupt_handler    /* 21 */
    j _interrupt_handler    /* 22 */
    j _interrupt_handler    /* 23 */
    j _panic_handler        /* 24: watchdog */
    j _panic_handler        /* 25: cache error */
    j _interrupt_handler    /* 26: memprot (desligado na sua config) */
    j _interrupt_handler    /* 27: stack guard (desligado na sua config) */
    j _interrupt_handler    /* 28 */
    j _interrupt_handler    /* 29 */
    j _interrupt_handler    /* 30 */
    j _interrupt_handler    /* 31 */
    .option pop
    .size meu_vector_table, .-meu_vector_table

    /* ===== o interceptador da linha 4 (usado so no PASSO 2) ===== */
    .section .iram1, "ax"
    .global meu_intercept
    .type meu_intercept, @function
meu_intercept:
    addi sp, sp, -16        /* abre espaco temporario */
    sw   t0, 0(sp)          /* salva t0 e t1 (vou usar eles) */
    sw   t1, 4(sp)
    la   t0, tick_count     /* pega o endereco do contador */
    lw   t1, 0(t0)
    addi t1, t1, 1          /* tick_count++ */
    sw   t1, 0(t0)
    lw   t0, 0(sp)          /* restaura t0 e t1 EXATAMENTE como estavam */
    lw   t1, 4(sp)
    addi sp, sp, 16         /* devolve o sp exatamente como estava */
    j    _interrupt_handler /* repassa pro handler original (tick normal) */
