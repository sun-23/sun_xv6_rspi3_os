/* Context switch
 * 
 *   void swtch(struct context **old, struct context *new);
 * 
 * Save current register context on the stack, 
 * creating a struct context, and save its address in *old.
 * Switch stacks to new and pop previously-saved registers.
 */
.global swtch

swtch:
    /* TODO: Your code here. */
    # Save old callee-saved registers
    str	x30, [sp, #-8]!
    str	x29, [sp, #-8]!
    str	x28, [sp, #-8]!
    str	x27, [sp, #-8]!
    str	x26, [sp, #-8]!
    str	x25, [sp, #-8]!
    str	x24, [sp, #-8]!
    str	x23, [sp, #-8]!
    str	x22, [sp, #-8]!
    str	x21, [sp, #-8]!
    str	x20, [sp, #-8]!
    str	x19, [sp, #-8]!
    str	x18, [sp, #-8]!
    str	x17, [sp, #-8]!
    str	x16, [sp, #-8]!

    # Switch stacks
    mov x19, sp
    
    # *old = sp = struct context
    str x19, [x0]
    mov sp, x1

    # Load new callee-saved registers
    ldr x16, [sp], #8
    ldr x17, [sp], #8
    ldr x18, [sp], #8
    ldr x19, [sp], #8
    ldr x20, [sp], #8
    ldr x21, [sp], #8
    ldr x22, [sp], #8
    ldr x23, [sp], #8
    ldr x24, [sp], #8
    ldr x25, [sp], #8
    ldr x26, [sp], #8
    ldr x27, [sp], #8
    ldr x28, [sp], #8
    ldr x29, [sp], #8
    ldr x30, [sp], #8

    ret