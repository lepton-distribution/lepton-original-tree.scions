/*
 * Banc KAL — lecture des registres R4-R11 courants (ARMv7-M).
 * void kal_regs_read(uint32_t out[8]);
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
