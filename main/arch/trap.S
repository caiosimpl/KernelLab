// arch/trap.S ÔÇö tabela de vetores propria do KernelLab
    
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
    addi sp, sp, -144        //abre espaco temporario
    sw   ra, 0(sp)
    sw   t0, 16(sp)
    sw   t1, 20(sp)
    sw   t2, 24(sp)

    addi t0, sp, 144       //salva o sp original em t0
    sw   t0, 4(sp)         //salva sp original no frame

    sw   s0, 28(sp)
    sw   s1, 32(sp)
    sw   a0, 36(sp)
    sw   a1, 40(sp)
    sw   a2, 44(sp)
    sw   a3, 48(sp)
    sw   a4, 52(sp)
    sw   a5, 56(sp)
    sw   a6, 60(sp)
    sw   a7, 64(sp)
    sw   s2, 68(sp)
    sw   s3, 72(sp)
    sw   s4, 76(sp)
    sw   s5, 80(sp)
    sw   s6, 84(sp)
    sw   s7, 88(sp)
    sw   s8, 92(sp)
    sw   s9, 96(sp)
    sw   s10, 100(sp)
    sw   s11, 104(sp)
    sw   t3, 108(sp)
    sw   t4, 112(sp)
    sw   t5, 116(sp)
    sw   t6, 120(sp)

    csrr t0, mepc       //salvamos o conteudos de mepc e mstatus
    sw   t0, 124(sp)
    csrr t0, mstatus
    sw   t0, 128(sp)

// incrementa tick
   la    t0, tick_count
   lw    t1, 0(t0)
   addi  t1, t1, 1
   sw    t1, 0(t0)

//limpa a interrupcao do systimer (TARGET0)
   li    t0, 0x6002306C   // SYSTIMER_INT_CLR_REG = 0x60023000 + 0x6c
   li    t1, 1            // SYSTIMER_TARGET0_INT_CLR = BIT(0)
   sw    t1, 0(t0)


	mv	 a0, sp 		  // passa o sp atual (frame da tarefa que sai)
	call pick_next_sp	  // a0 = sp do proximo (frama da tarefa que entra)
	mv   sp, a0      	  // troca para o frame do proximo

// restasura

    lw   t0, 124(sp)
    csrw mepc, t0
    lw   t0, 128(sp)
    csrw mstatus, t0

    lw   ra, 0(sp)      //comecamos a restaurar o contexto
    lw   t0, 16(sp)
    lw   t1, 20(sp)
    lw   t2, 24(sp)
    lw   s0, 28(sp)
    lw   s1, 32(sp)
    lw   a0, 36(sp)
    lw   a1, 40(sp)
    lw   a2, 44(sp)
    lw   a3, 48(sp)
    lw   a4, 52(sp)
    lw   a5, 56(sp)
    lw   a6, 60(sp)
    lw   a7, 64(sp)
    lw   s2, 68(sp)
    lw   s3, 72(sp)
    lw   s4, 76(sp)
    lw   s5, 80(sp)
    lw   s6, 84(sp)
    lw   s7, 88(sp)
    lw   s8, 92(sp)
    lw   s9, 96(sp)
    lw   s10, 100(sp)
    lw   s11, 104(sp)
    lw   t3, 108(sp)
    lw   t4, 112(sp)
    lw   t5, 116(sp)
    lw   t6, 120(sp)

    addi sp, sp, 144

   mret

