#include <stdint.h>
#include "vm.h"
#include "arm.h"
#include "string.h"
#include "console.h"
#include "kalloc.h"
#include "trap.h"
#include "timer.h"
#include "spinlock.h"

struct do_once
{
    int count;
    struct spinlock lock;
};

struct do_once alloc_once = {0};
struct do_once bss_clear = {0};

void
main()
{
    /*
     * Before doing anything else, we need to ensure that all
     * static/global variables start out zero.
     * The edata and end are defined in kern/linker.ld. vectors is defined in kern/vector.S
     */

    extern char edata[], end[], vectors[];

    /*
     * Determine which functions in main can only be
     * called once, and use lock to guarantee this.
     */
    /* TODO: Your code here. */
    acquire(&bss_clear.lock);
    if (!bss_clear.count) {
        bss_clear.count = 1;
        /* TODO: Use `memset` to clear the BSS section of our program. */
        memset(edata, 0, end - edata); 
    }
    release(&bss_clear.lock);
   
    /* TODO: Use `cprintf` to print "hello, world\n" */
    console_init();

    acquire(&alloc_once.lock);
    if (!alloc_once.count) {
        alloc_once.count = 1;
        alloc_init();
        // check_free_list();
        cprintf("Allocator: Init success.\n");
    }
    release(&alloc_once.lock);
    irq_init();

    lvbar(vectors);
    timer_init();

    cprintf("CPU %d: Init success.\n", cpuid());

    while (1) ;
}
