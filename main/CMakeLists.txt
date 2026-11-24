idf_component_register(SRCS "main.c" "switch.S" "tarefa.c" "process_table.c"
"arch/trap.S" "schduller/round_robin.c" "tests/tests.c" "kernel_timer.c" 
"arch/bootstrap_probe.S" "arch/irq_control.c"
                       INCLUDE_DIRS "." "tests" "arch" "schduller")
