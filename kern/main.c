#include <stdint.h>

#include "string.h"
#include "console.h"
#include "kalloc.h"
#include "vm.h"

void
main()
{
    /*
     * Before doing anything else, we need to ensure that all
     * static/global variables start out zero.
     * The edata and end are defined in kern/linker.ld.
     */
    extern char edata[], end[];

    /* TODO: Use `memset` to clear the BSS section of our program. */
    memset(edata, 0, end - edata);    
    /* TODO: Use `cprintf` to print "hello, world\n" */
    console_init();
    alloc_init();
    cprintf("Allocator: Init success.\n");
    check_free_list();
    vm_test();
    while (1) ;
}
