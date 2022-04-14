#include <stdint.h>
#include "vm.h"
#include "arm.h"
#include "string.h"
#include "console.h"
#include "kalloc.h"
#include "trap.h"
#include "timer.h"
#include "spinlock.h"

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

    /* TODO: Use `memset` to clear the BSS section of our program. */
    memset(edata, 0, end - edata);    
    /* TODO: Use `cprintf` to print "hello, world\n" */
    console_init();
    alloc_init();
    cprintf("Allocator: Init success.\n");
    check_free_list();
    // vm_test();

    irq_init();

    lvbar(vectors);
    timer_init();

    cprintf("CPU %d: Init success.\n", cpuid());

    while (1) ;
}
