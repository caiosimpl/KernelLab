
.section .bss
.balign 16

# Contexto do hospedeiro FreeRTOS.
#
#  0  ra
#  4  sp
#  8  gp
# 12  tp
# 16  s0
# 20  s1
# 24  s2
# ...
# 60  s11
# 64  mstatus original
# 68  mtvec original
#
# 80 bytes reservados.
probe_host_ctx:
    .space 80


.section .iram1, "ax"
.balign 4

.global kernel_probe_run
.type kernel_probe_run, @function

# void kernel_probe_run(uint32_t *frame)
#
# Entrada:
#   a0 = endereco do frame preemptivo
#
# Saida:
#   Retorna ao chamador depois que
#   kernel_probe_finish restaura
#   o contexto hospedeiro.
kernel_probe_run:

    # 1. Captura mstatus e desabilita MIE
    #    em uma unica instrucao.
    csrrci t1, mstatus, 8

    # 2. Endereco do contexto hospedeiro.
    la t0, probe_host_ctx

    # 3. Preserva o estado original de mstatus.
    sw t1, 64(t0)

    csrr t1, mtvec
    sw   t1, 68(t0)

    # 4. Salva os registradores necessarios
    #    para retornar ao chamador C.
    sw ra, 0(t0)
    sw sp, 4(t0)
    sw gp, 8(t0)
    sw tp, 12(t0)

    sw s0, 16(t0)
    sw s1, 20(t0)
    sw s2, 24(t0)
    sw s3, 28(t0)
    sw s4, 32(t0)
    sw s5, 36(t0)
    sw s6, 40(t0)
    sw s7, 44(t0)
    sw s8, 48(t0)
    sw s9, 52(t0)
    sw s10, 56(t0)
    sw s11, 60(t0)

    # insere vetor KernelLab
    la t1, meu_vector_table
    ori t1, t1, 1
    csrw mtvec, t1

    # 5. A CPU passa a utilizar a pilha do contexto experimental.
    mv sp, a0

    # 6. Restauracao do frame e mret.
    j restore_preempt_frame


.global kernel_probe_task
.type kernel_probe_task, @function

# Tarefa experimental.
# Nao utiliza chamadas C, FreeRTOS ou TLS.
kernel_probe_task:

    # Captura mtvec antes de alterar t1.
    csrr t1, mtvec

    la t0, probe_mtvec_seen
    sw t1, 0(t0)

    # Teste anterior de t2.
    la t0, probe_t2_seen
    sw t2, 0(t0)

    # Teste do marcador.
    la t0, probe_marker
    li t1, 0x4B4C4142
    sw t1, 0(t0)

    j kernel_probe_finish

.global kernel_probe_finish
.type kernel_probe_finish, @function

kernel_probe_finish:

    # 1. Garante MIE desabilitado antes
    #    de restaurar os registradores.
    csrci mstatus, 8

    # 2. Recupera o endereco do contexto.
    la t0, probe_host_ctx

    lw t1, 68(t0)
    csrw mtvec, t1

    # 3. Restaura o contexto hospedeiro.
    lw ra, 0(t0)
    lw sp, 4(t0)
    lw gp, 8(t0)
    lw tp, 12(t0)

    lw s0, 16(t0)
    lw s1, 20(t0)
    lw s2, 24(t0)
    lw s3, 28(t0)
    lw s4, 32(t0)
    lw s5, 36(t0)
    lw s6, 40(t0)
    lw s7, 44(t0)
    lw s8, 48(t0)
    lw s9, 52(t0)
    lw s10, 56(t0)
    lw s11, 60(t0)

    # 4. Recupera mstatus original.
    #    Fazemos isso somente depois
    #    de restaurar SP, GP e TP.
    lw t1, 64(t0)
    csrw mstatus, t1

    # 5. Retorna ao chamador C.
    ret

