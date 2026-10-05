/*
 * Banc KAL — lecture des registres R4-R11 courants (ARMv6-M, Thumb-1 : R8-R11 par MOV).
 * void kal_regs_read(uint32_t out[8]);
 * uint32_t kal_cpu_primask(void);   registre PRIMASK (b0 : interruptions masquées)
 */
    .syntax unified
    .thumb
    .text

    .global kal_regs_read
    .thumb_func
    .type kal_regs_read, %function
kal_regs_read:
    stmia   r0!, {r4-r7}
    mov     r1, r8
    str     r1, [r0, #0]
    mov     r1, r9
    str     r1, [r0, #4]
    mov     r1, r10
    str     r1, [r0, #8]
    mov     r1, r11
    str     r1, [r0, #12]
    bx      lr
    .size kal_regs_read, . - kal_regs_read

    .global kal_cpu_primask
    .thumb_func
    .type kal_cpu_primask, %function
kal_cpu_primask:
    mrs     r0, primask
    bx      lr
    .size kal_cpu_primask, . - kal_cpu_primask
