#include <stdint.h>
#include "types.h"
#include "mmu.h"
#include "string.h"
#include "memlayout.h"
#include "console.h"

#include "vm.h"
#include "kalloc.h"

/* 
 * Given 'pgdir', a pointer to a page directory, pgdir_walk returns
 * a pointer to the page table entry (PTE) for virtual address 'va'.
 * This requires walking the four-level page table structure.
 *
 * The relevant page table page might not exist yet.
 * If this is true, and alloc == false, then pgdir_walk returns NULL.
 * Otherwise, pgdir_walk allocates a new page table page with kalloc.
 *   - If the allocation fails, pgdir_walk returns NULL.
 *   - Otherwise, the new page is cleared, and pgdir_walk returns
 *     a pointer into the new page table page.
 */

static uint64_t *
pgdir_walk(uint64_t *pgdir, const void *va, int64_t alloc)
{
    /* TODO: Your code here. */
    // check bits 48-63 shoud be 0xffff or 0 
    uint64_t sign = ((uint64_t)va >> 48) & 0xFFFF;
    if (sign != 0 && sign != 0xFFFF) return NULL;

    uint64_t *pt1 = &(pgdir[L0X((uint64_t)va)]);
    if (!(*pt1 & (PTE_P)))
    {
        void *p;
        if (!alloc || (p = (uint64_t)kalloc()) == 0)
            return 0;

        // V2P because *ptX must hold physical address
        // PTE_P to valid
        // PTE_TABLE to table to get address of next pagetable
        // and not block (we not implement block)
        *pt1 = V2P(p) | PTE_TABLE | PTE_P ;
    }
    //get address of next page table level
    //P2V to get virtual address because in entry.c 
    //we map physical address [0x0, 0x4000_0000) to 
    //virtual address [0xFFFF_0000_0000_0000, 0XFFFF_0000_4000_0000)
    //and kernel start in virtual address 0xFFFF_0000_0000_0000
    pgdir = (uint64_t)(P2V(PTE_ADDR(*pt1)));

    uint64_t *pt2 = &(pgdir[L1X((uint64_t)va)]);
    if (!(*pt2 & (PTE_P)))
    {
        void *p;
        if (!alloc || (p = (uint64_t)kalloc()) == 0)
            return 0;
        *pt2 = V2P(p) | PTE_TABLE | PTE_P ;
    }
    pgdir = (uint64_t)(P2V(PTE_ADDR(*pt2)));

    uint64_t *pt3 = &(pgdir[L2X((uint64_t)va)]);
    if (!(*pt3 & (PTE_P)))
    {
        void *p;
        if (!alloc || (p = (uint64_t)kalloc()) == 0)
            return 0;
        *pt3 = V2P(p) | PTE_TABLE | PTE_P ;
    }
    //get address of page table
    pgdir = (uint64_t)(P2V(PTE_ADDR(*pt3)));

    //return address of page table index
    return &pgdir[L3X(va)];
}

/*
 * Create PTEs for virtual addresses starting at va that refer to
 * physical addresses starting at pa. va and size might **NOT**
 * be page-aligned.
 * Use permission bits perm|PTE_P|PTE_TABLE|PTE_AF for the entries.
 *
 * Hint: call pgdir_walk to get the corresponding page table entry
 */

static int
map_region(uint64_t *pgdir, void *va, uint64_t size, uint64_t pa, int64_t perm)
{
    /* TODO: Your code here. */
    uint64_t* va_current, * va_end;
    uint64_t* pte;
    va_current = (uint64_t*)PTE_ADDR(va);
    va_end = (uint64_t*)PTE_ADDR(va + size - 1);
    while (1) {
        if ((pte = pgdir_walk(pgdir, va_current, 1)) == 0)//let pte be the pointer of the end of the table page
            return -1;
        if (*pte & PTE_P)
            panic("this pte %llx is mapped\n", *pte);
        *pte = PTE_ADDR(pa) | perm | PTE_P | PTE_TABLE | PTE_AF ;//set pte value
        if (va_current == va_end)
            break;
        va_current += PGSIZE;
        pa += PGSIZE;
    }
    return 0;
}

/* 
 * Free a page table.
 *
 * Hint: You need to free all existing PTEs for this pgdir.
 */

void
vm_free(uint64_t *pgdir, int level)
{
    /* TODO: Your code here. */
    for (int i = 0; i < 512; i++) {
        uint64_t pte = pgdir[i];
        if (pte & PTE_P) {
            if (level < 3)
                //P2V because pte holds physical address 
                //kernel run in virtul address must use virtual address.
                vm_free((uint64_t*)(P2V(PTE_ADDR(pte))), level + 1);
            else if (level == 3) {
                //P2V because pte holds physical address 
                //kernel run in virtul address must use virtual address.
                kfree((char*)P2V(PTE_ADDR(pte)));
            }
        }
    }
    if (level < 3)
        kfree((char*)pgdir); 
    //no P2V bacause in vm_free previous level 
    //we call this vm_free level with virtual address

    return;
}


void
vm_test()
{
    *((int64_t*)P2V(0)) = 0xac;
    char* p = kalloc();
    memset(p, 0, PGSIZE);
    map_region((uint64_t*)p, (void*)0x1000, PGSIZE, 0, 0);

    //V2P beacuse ttbr0_el1 must hold physical address of page table
    asm volatile("msr ttbr0_el1, %[x]": : [x] "r"(V2P(p)));

    if (*((int64_t*)0x1000) == 0xac) {
        cprintf("Test_Map_Region Pass!\n");
    }
    else {
        cprintf("Test_Map_Region Fail!\n");
    }
}