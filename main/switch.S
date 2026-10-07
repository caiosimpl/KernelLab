#Switch.S - troca de contexto (no momento cooperativa) RISC-V

# void context_switch(tct_t *atual, tcb_t *outra) {
# 	a0 = ponteiro pro TCB da tarefa que SAI (onde salvar o sp)
#	a1 = ponteiro pro TCB da tarefa que ENTRA (onde ler o sp)

#}

# O campo sp ├® o PRIMEIRO da struct tcb_t enta o offset ├® 0
# typedef struct {uint32_t *sp, uint32_t *stack_base} tcb_t;


.section .text
.global context_switch
.align 2


context_switch:

	# Salva o contexto do processo atual

	addi sp, sp, -64   # abre 64 bytes na stack
    sw  ra,  0(sp)     # guarda de onde voltar
    sw  s0,  4(sp)
    sw  s1,  8(sp)
    sw  s2,  12(sp)
    sw  s3,  16(sp)
    sw  s4,  20(sp)
    sw  s5,  24(sp)
    sw  s6,  28(sp)
    sw  s7,  32(sp)
    sw  s8,  36(sp)
    sw  s9,  40(sp)
    sw  s10, 44(sp)
    sw  s11, 48(sp)

	sw  sp, 0(a0)   # atual->sp = sp (salva o sp da TCB que sai)

	lw  sp, 0(a1)   # sp = proxima->sp (le o sp TCB da tarefa que entra)


	#restaura o contexo da proxima tarefa
	lw   ra,  0(sp)
    lw   s0,  4(sp)
    lw   s1,  8(sp)
    lw   s2,  12(sp)
    lw   s3,  16(sp)
    lw   s4,  20(sp)
    lw   s5,  24(sp)
    lw   s6,  28(sp)
    lw   s7,  32(sp)
    lw   s8,  36(sp)
    lw   s9,  40(sp)
    lw   s10, 44(sp)
    lw   s11, 48(sp)
    addi sp, sp, 64

    ret

