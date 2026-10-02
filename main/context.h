#ifndef CONTEXT_H
#define CONTEXT_H
#include <stdio.h>

struct context {
    uint32_t ra;
    uint32_t sp;
    uint32_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
};

void context_switch(struct context *old, struct context *new);

#endif
