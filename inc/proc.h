#ifndef INC_PROC_H
#define INC_PROC_H

#include <stddef.h>

#include "arm.h"
#include "trap.h"

#define NCPU   4        /* maximum number of CPUs */
#define NPROC 64        /* maximum number of processes */
#define NOFILE 16       /* open files per process */
#define KSTACKSIZE 4096 /* size of per-process kernel stack */

#define thiscpu (&cpus[cpuid()])

struct cpu {
    struct context *scheduler;  /* swtch() here to enter scheduler */
    struct proc *proc;          /* The process running on this cpu or null */
};

extern struct cpu cpus[NCPU];

/*
 * Saved registers for kernel context switches.
 * Don't need to save X1-X15 since accorrding to 
 * the x86 convention it is the caller to save them.
 * Contexts are stored at the top of the stack they describe,
 * the stack pointer is the address of the context.
 * The layout of the context matches the layout of the stack in swtch.S
 */
struct context {
    /* TODO: Your code here. */
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
    uint64_t x30;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct proc {
    uint64_t sz;             /* Size of process memory (bytes)          */
    uint64_t *pgdir;         /* Page table                              */
    char *kstack;            /* Bottom of kernel stack for this process */
    enum procstate state;    /* Process state                           */
    int pid;                 /* Process ID                              */
    struct proc *parent;     /* Parent process                          */
    struct trapframe *tf;    /* Trapframe for current syscall           */
    struct context *context; /* swtch() here to run process             */
    void *chan;              /* If non-zero, sleeping on chan           */
    int killed;              /* If non-zero, have been killed           */
    char name[16];           /* Process name (debugging)                */

    struct file *ofile[NOFILE];  /* Open files */
    struct inode *cwd;           /* Current directory */
};

static inline struct proc *
thisproc()
{
    return thiscpu->proc;
}

void proc_init();
void user_init();
void user_idle_init();
void scheduler();

void yield();
void exit();
int fork();
int wait();

void sleep();
void wakeup();

int growproc(int n);

int sys_yield();
size_t sys_brk();
int sys_clone();
int sys_wait4();
int sys_exit();


#endif
