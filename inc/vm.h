#ifndef KERN_VM_H
#define KERN_VM_H

#include "memlayout.h"

void vm_free(uint64_t *, int);
void vm_test();

#endif /* !KERN_VM_H */