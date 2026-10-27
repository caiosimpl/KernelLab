// arch/trap.S — tabela de vetores propria do KernelLa
    
	.global _interrupt_handler
    .global _panic_handler

    // a NOSSA tabela de vetores
    .section .iram1, "ax"
    .balign 0x100                 //OBRIGATORIO: alinhar em 256
    .option push
    .option norvc                 //OBRIGATORIO: cada 'j' = pulo de 4 bytes
    .global meu_vector_table
    .type meu_vector_table, @function

// as linhas que nao estao comentadas sao outras interrupcoes de perifericos do esp (cabo tipo C, wifi, bluetooth etc...)
meu_vector_table:
    j _panic_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j meu_intercept         // 4: esse cara eh onde a interrupcao de time bate e gente "redireciona"
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler    
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _interrupt_handler
    j _panic_handler
    j _panic_handler
    j _interrupt_handler    // 26: memprot (desligado no MENUCONFIG)
    j _interrupt_handler    // 27: stack guard (desligado no MENUCONFIG)
    j _interrupt_handler
    j _interrupt_handler 
    j _interrupt_handler
    j _interrupt_handler
    .option pop
    .size meu_vector_table, .-meu_vector_table

    // interceptador da linha 4
    .section .iram1, "ax"
    .global meu_intercept
    .type meu_intercept, @function


// esse cara tem que ser uma troca de contexto completa com os 32 registradores
// Temos que estudar como faze-la com os 32 regostradores e devolver o mret pro lugar certo
meu_intercept:
    addi sp, sp, -16        //abre espaco temporario
    sw   t0, 0(sp)          // salva t0 e t1 (vou usar eles)
    sw   t1, 4(sp)
    la   t0, tick_count     // pega o endereco do contador
    lw   t1, 0(t0)
    addi t1, t1, 1          // tick_count++
    sw   t1, 0(t0)
    lw   t0, 0(sp)          // restaura t0 e t1 EXATAMENTE como estavam
    lw   t1, 4(sp)
    addi sp, sp, 16         // devolve o sp exatamente como estava
    j    _interrupt_handler // repassa pro handler original (tick normal)
