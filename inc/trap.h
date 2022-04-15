#ifndef INC_TRAP_H
#define INC_TRAP_H

#include <stdint.h>

struct trapframe {
    /* TODO: Design your own trapframe layout here. */
    // save register value to struct (stack) asm push from botom to top 
    /* example 
        push x30 
        push x29 
        .
        .
        . 
        push SPSR_EL1
        push ELR_EL1 
        # cannot push XXX_ELX direcly 
        # you should store to register aka x2 and push x2 register
    */
    // restore struct to register asm pop from top to bottom 
    /* example 
        pop ELR_EL1 
        pop SPSR_EL1 
        .
        .
        . 
        pop x29
        pop x30 
        # cannot pop XXX_ELX direcly 
        # you should pop to register aka x2 and write data 
        # from register x2 to XXX_ELX
    */ 

   //support musl
   __uint128_t Q0;
    uint64_t TPIDR_EL0;
    uint64_t TPIDR_EL0_COPY;

    uint64_t ELR_EL1; //top
    uint64_t SPSR_EL1;
    uint64_t SP_EL0;

    uint64_t x0;
    uint64_t x1;
    uint64_t x2;
    uint64_t x3;
    uint64_t x4;
    uint64_t x5;
    uint64_t x6;
    uint64_t x7;
    uint64_t x8;
    uint64_t x9;
    uint64_t x10;
    uint64_t x11;
    uint64_t x12;
    uint64_t x13;
    uint64_t x14;
    uint64_t x15;
    uint64_t x16;
    uint64_t x17;
    uint64_t x18;
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t x29;
    uint64_t x30; //bottom
};

void trap(struct trapframe *);
void irq_init();
void irq_error();

#endif
