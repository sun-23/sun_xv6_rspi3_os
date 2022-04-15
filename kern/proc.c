#include "proc.h"
#include "spinlock.h"
#include "console.h"
#include "kalloc.h"
#include "trap.h"
#include "string.h"
#include "vm.h"
#include "mmu.h"
#include "sd.h"
#include "file.h"
#include "log.h"

struct {
    struct proc proc[NPROC];
    struct spinlock lock;
} ptable;

static struct proc *initproc;

// int nextpid = 1;
struct {
    int nextpid;
    struct spinlock lock;
} nextpid = { 1 };

void forkret();
extern void trapret();
void swtch(struct context **, struct context *);

int
alloc_pid()
{
    acquire(&nextpid.lock);
    int pid = nextpid.nextpid;
    nextpid.nextpid++;
    release(&nextpid.lock);
    return pid;
}

/*
 * Initialize the spinlock for ptable to serialize the access to ptable
 */
void
proc_init()
{
    /* TODO: Your code here. */
    initlock(&ptable.lock, "proc table");
    initlock(&nextpid.lock, "nextpid");
}

/*
 * Look through the process table for an UNUSED proc.
 * If found, change state to EMBRYO and initialize
 * state (allocate stack, clear trapframe, set context for switch...)
 * required to run in the kernel. Otherwise return 0.
 */
static struct proc *
proc_alloc()
{
    struct proc *p;
    /* TODO: Your code here. */
    int found = 0;
    acquire(&ptable.lock);
    for (p = ptable.proc;p < ptable.proc + NPROC;p++) {
        if (p->state == UNUSED) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        release(&ptable.lock);
        return NULL;
    }

    // kstack
    char* sp = kalloc();
    if (sp == NULL) {
        p->state = UNUSED;
        return 0;
    }
    p->kstack = sp;
    sp += KSTACKSIZE;
    // trapframe
    sp -= sizeof(*(p->tf));
    p->tf = (struct trapframe*)sp;
    memset(p->tf, 0, sizeof(*(p->tf)));
    // trapret
    sp -= 8;
    *(uint64_t*)sp = (uint64_t)trapret;
    // sp
    sp -= 8;
    *(uint64_t*)sp = (uint64_t)p->kstack + KSTACKSIZE;
    // context
    sp -= sizeof(*(p->context));
    p->context = (struct context*)sp;
    memset(p->context, 0, sizeof(*(p->context)));
    p->context->x30 = (uint64_t)forkret + 8;

    // other settings
    p->pid = alloc_pid();
    p->state = EMBRYO;

    release(&ptable.lock);

    return p;
}

/*
 * Set up first user process(Only used once).
 * Set trapframe for the new process to run
 * from the beginning of the user process determined 
 * by uvm_init
 */
void
user_init()
{
    struct proc* p;
    /* for why our symbols differ from xv6, please refer https://stackoverflow.com/questions/10486116/what-does-this-gcc-error-relocation-truncated-to-fit-mean */
    extern char _binary_obj_user_initcode_start[], _binary_obj_user_initcode_size[];

    /* TODO: Your code here. */
    p = proc_alloc();
    initproc = p;

    if ((p->pgdir = pgdir_init()) == 0) {
        panic("user_init: out of memory?");
    }

    uvm_init(p->pgdir, _binary_obj_user_initcode_start, (int)_binary_obj_user_initcode_size);

    p->sz = PGSIZE;
    memset(p->tf, 0, sizeof(*p->tf));

    p->tf->SPSR_EL1 = 0x00;
    p->tf->SP_EL0 = PGSIZE;
    p->tf->x30 = 0;
    p->tf->ELR_EL1 = 0;

    strncpy(p->name, "initcode", sizeof(p->name));
    p->state = RUNNABLE;
    p->cwd = namei("/");
    p->sz = PGSIZE;
}

void user_idle_init()
{
    struct proc* p;
    /* for why our symbols differ from xv6, please refer https://stackoverflow.com/questions/10486116/what-does-this-gcc-error-relocation-truncated-to-fit-mean */
    extern char _binary_obj_user_initcode_start[], _binary_obj_user_initcode_size[];

    p = proc_alloc();

    if (p == NULL) {
        panic("user_init: cannot allocate a process");
    }
    if ((p->pgdir = pgdir_init()) == NULL) {
        panic("user_init: cannot allocate a pagetable");
    }

    initproc = p;

    //start in loop
    uvm_init(p->pgdir, _binary_obj_user_initcode_start + (7 << 2), (long)(_binary_obj_user_initcode_size - (7 << 2)));

    // tf
    memset(p->tf, 0, sizeof(*(p->tf)));
    p->tf->SPSR_EL1 = 0;
    p->tf->SP_EL0 = PGSIZE;
    p->tf->x30 = 0;
    p->tf->ELR_EL1 = 0;

    p->state = RUNNABLE;
    p->sz = PGSIZE;
}

/*
 * Per-CPU process scheduler
 * Each CPU calls scheduler() after setting itself up.
 * Scheduler never returns.  It loops, doing:
 *  - choose a process to run
 *  - swtch to start running that process
 *  - eventually that process transfers control
 *        via swtch back to the scheduler.
 */
void
scheduler()
{
    struct proc *p;
    struct cpu *c = thiscpu;
    c->proc = NULL;
    
    for (;;) {
        /* Loop over process table looking for process to run. */
        /* TODO: Your code here. */
        acquire(&ptable.lock);
        for (p = ptable.proc; p < ptable.proc + NPROC; p++) {
            if (p->state == RUNNABLE) {
                c->proc = p;
                uvm_switch(p);
                p->state = RUNNING;
                // cprintf("scheduler: process id %d takes the cpu %d\n", p->pid, cpuid());
                swtch(&c->scheduler, p->context);

                // back
                c->proc = NULL;
            }

        }
        release(&ptable.lock);
    }
}

/*
 * Enter scheduler.  Must hold only ptable.lock
 */
void
sched()
{
    /* TODO: Your code here. */
    struct proc* p = thiscpu->proc;
    if (!holding(&ptable.lock)) {
        panic("sched: not holding ptable lock");
    }

    if (p->state == RUNNING) {
        panic("sched: process running");
    }

    swtch(&p->context, thiscpu->scheduler);
}

/*
 * A fork child will first swtch here, and then "return" to user space.
 */
void
forkret()
{
    /* TODO: Your code here. */
    //release ptable.lok that is aquired in scheduler
    release(&ptable.lock);

    if (thiscpu->proc->pid == 1) {
        initlog(ROOTDEV);
        // sd_test();
        cprintf("init the log successfully\n");
#ifdef TEST_FILE_SYSTEM
        test_file_system();
#endif
    }
    return;
}

void wakeup_withlock(void* chan) {
    for (struct proc* p = ptable.proc; p < ptable.proc + NPROC; p++) {
        if (p->state == SLEEPING && p->chan == chan) {
            p->state = RUNNABLE;
        }
    }
}

/*
 * Exit the current process.  Does not return.
 * An exited process remains in the zombie state
 * until its parent calls wait() to find out it exited.
 */
void
exit()
{
    struct proc *p = thiscpu->proc;
    /* TODO: Your code here. */
    if (p == initproc) {
        panic("exit: init process shall not exit!");
    }

    for (int fd = 0; fd < NOFILE; fd++) {
        if (thisproc()->ofile[fd]) {
            fileclose(thisproc()->ofile[fd]);
            thisproc()->ofile[fd] = 0;
        }
    }
    iput(thisproc()->cwd);
    thisproc()->cwd = 0;
    acquire(&ptable.lock);
    wakeup_withlock(p->parent);
    for (struct proc* p = ptable.proc;p < ptable.proc + NPROC; p++) {
        if (p->parent == thisproc()) {
            p->parent = initproc;
            if (p->state == ZOMBIE) {
                wakeup_withlock(p->parent);
            }
        }
    }
    p->state = ZOMBIE;
    sched();

    // never exit
    panic("exit: shall not return");
}

/*
 * Atomically release lock and sleep on chan.
 * Reacquires lock when awakened.
 */
void
sleep(void *chan, struct spinlock *lk)
{
    /* TODO: Your code here. */
    struct proc* p = thiscpu->proc;

    if (p == 0) {
        panic("sleep");
    }


    if (lk != &ptable.lock) {
        acquire(&ptable.lock);
        release(lk);
    }

    p->chan = chan;
    p->state = SLEEPING;


    sched();
    p->chan = 0;

    if (lk != &ptable.lock) {
        release(&ptable.lock);
        acquire(lk);
    }
}

/* Wake up all processes sleeping on chan. */
void
wakeup(void *chan)
{
    /* TODO: Your code here. */
    struct proc* p;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == SLEEPING && p->chan == chan) {
            p->state = RUNNABLE;
        }
    }
    release(&ptable.lock);
}

/* Give up CPU. */
void
yield()
{
    /* TODO: Your code here. */
    acquire(&ptable.lock);
    struct proc* p = thiscpu->proc;
    p->state = RUNNABLE;
    // cprintf("yield: process id %d gives up the cpu %d\n", p->pid, cpuid());
    sched();
    release(&ptable.lock);
}

/*
 * Create a new process copying p as the parent.
 * Sets up stack to return as if from system call.
 * Caller must set state of returned proc to RUNNABLE.
 */
int
fork()
{
    /* TODO: Your code here. */
    uint32_t i, pid;
    struct proc* np;
    struct trapframe* tmptf;

    // Allocate process.
    if ((np = proc_alloc()) == 0) {
        return -1;
    }

    // Copy process state from p.
    if ((np->pgdir = copyuvm(thisproc()->pgdir, thisproc()->sz)) == 0) {
        kfree(np->kstack);
        np->kstack = 0;
        np->state = UNUSED;
        return -1;
    }

    np->sz = thisproc()->sz;
    np->parent = thisproc();
    memmove(np->tf, thisproc()->tf, sizeof(struct trapframe));

    // Clear r0 so that fork returns 0 in the child.
    np->tf->x0 = 0;

    for (i = 0; i < NOFILE; i++) {
        if (thisproc()->ofile[i]) {
            np->ofile[i] = filedup(thisproc()->ofile[i]);
        }
    }

    np->cwd = idup(thisproc()->cwd);

    pid = np->pid;
    np->state = RUNNABLE;
    strncpy(np->name, thisproc()->name, sizeof(thisproc()->name));

    return pid;
}

/*
 * Wait for a child process to exit and return its pid.
 * Return -1 if this process has no children.
 */
int
wait()
{
    /* TODO: Your code here. */
    struct proc* p;
    int havekids, pid;

    acquire(&ptable.lock);

    for (;;) {
        // Scan through table looking for zombie children.
        havekids = 0;

        for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
            if (p->parent != thisproc()) {
                continue;
            }

            havekids = 1;

            if (p->state == ZOMBIE) {
                // Found one.
                pid = p->pid;
                kfree(p->kstack);
                p->kstack = 0;
                vm_free(p->pgdir, 1);
                p->state = UNUSED;
                p->pid = 0;
                p->parent = 0;
                p->name[0] = 0;
                p->killed = 0;
                release(&ptable.lock);

                return pid;
            }
        }

        // No point waiting if we don't have any children.
        if (!havekids || thisproc()->killed) {
            release(&ptable.lock);
            return -1;
        }

        // Wait for children to exit.  (See wakeup1 call in proc_exit.)
        sleep(thisproc(), &ptable.lock);  //DOC: wait-sleep
    }
}

/*
 * Print a process listing to console.  For debugging.
 * Runs when user types ^P on console.
 * No lock to avoid wedging a stuck machine further.
 */
void
procdump()
{
    // panic("unimplemented");
    static char* states[] = {
        [UNUSED] "UNUSED  ",  [SLEEPING] "SLEEPING", [RUNNABLE] "RUNNABLE",
        [RUNNING] "RUNNING ", [ZOMBIE] "ZOMBIE  ",
    };

    cprintf("\n====== PROCESS DUMP ======\n");
    acquire(&ptable.lock);
    for (struct proc* p = ptable.proc; p < &ptable.proc[NPROC]; ++p) {
        if (p->state == UNUSED) continue;
        char* state =
            (p->state >= 0 && p->state < ARRAY_SIZE(states) && states[p->state])
                ? states[p->state]
                : "UNKNOWN ";
        cprintf("[%s] %d (%s)\n", state, p->pid, p->name);
    }
    cprintf("====== DUMP END ======\n\n");
    release(&ptable.lock);
}

int growproc(int n)
{
    uint32_t sz;
    sz = thisproc()->sz;

    if (n > 0) {
        if ((sz = allocuvm(thisproc()->pgdir, sz, sz + n)) == 0) {
            return -1;
        }

    }
    else if (n < 0) {
        if ((sz = deallocuvm(thisproc()->pgdir, sz, sz + n)) == 0) {
            return -1;
        }
    }

    thisproc()->sz = sz;
    uvm_switch(thisproc());
    return 0;
}