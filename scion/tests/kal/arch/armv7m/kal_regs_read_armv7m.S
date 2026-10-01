/*
 * Banc KAL — lecture des registres R4-R11 courants (ARMv7-M).
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
    stmia   r0, {r4-r11}
    bx      lr
    .size kal_regs_read, . - kal_regs_read

    .global kal_cpu_primask
    .thumb_func
    .type kal_cpu_primask, %function
kal_cpu_primask:
    mrs     r0, primask
    bx      lr
    .size kal_cpu_primask, . - kal_cpu_primask
