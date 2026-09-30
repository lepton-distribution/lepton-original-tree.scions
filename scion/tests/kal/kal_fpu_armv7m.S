/*
 * Banc KAL — registres FPU (ARMv7-M, FPv4-SP, ABI hard ; écart E3, embos-iar-vs-gcc.md).
 *
 * void kal_fpu_hold(const uint32_t pattern[16], uint32_t out[16], void (*wait)(void));
 *   charge S16-S31 avec pattern[0..15], appelle wait() (la tâche s'y bloque avec un contexte FPU
 *   actif : le micro-noyau sauvegarde un cadre étendu), puis écrit S16-S31 dans out[0..15].
 *   S16-S31 sont préservés par l'AAPCS : toute différence vient du contexte sauvegardé (KAL).
 * void kal_fpu_fill(uint32_t value);     S0-S31 = value (écrasement par un gestionnaire, un fils)
 * void kal_fpu_read(uint32_t out[16]);   out = S16-S31
 * void kal_fpu_set_fpscr(uint32_t v);
 * uint32_t kal_fpu_fpscr(void);
 * uint32_t kal_cpu_control(void);        registre CONTROL (bit 2 : FPCA, contexte FPU actif)
 * uint32_t kal_fpu_fpccr(void);          SCB FPCCR (bits 31 ASPEN, 30 LSPEN : lazy stacking)
 */
    .syntax unified
    .thumb
    .fpu fpv4-sp-d16
    .text

    .global kal_fpu_hold
    .thumb_func
    .type kal_fpu_hold, %function
kal_fpu_hold:
    push    {r4, r5, r6, lr}
    vpush   {s16-s31}
    mov     r4, r1                /* out */
    mov     r5, r2                /* wait */
    vldmia  r0, {s16-s31}
    blx     r5
    vstmia  r4, {s16-s31}
    vpop    {s16-s31}
    pop     {r4, r5, r6, pc}
    .size kal_fpu_hold, . - kal_fpu_hold

    .global kal_fpu_fill
    .thumb_func
    .type kal_fpu_fill, %function
kal_fpu_fill:
    vmov    s0, r0
    vmov    s1, r0
    vmov    s2, r0
    vmov    s3, r0
    vmov    s4, r0
    vmov    s5, r0
    vmov    s6, r0
    vmov    s7, r0
    vmov    s8, r0
    vmov    s9, r0
    vmov    s10, r0
    vmov    s11, r0
    vmov    s12, r0
    vmov    s13, r0
    vmov    s14, r0
    vmov    s15, r0
    vmov    s16, r0
    vmov    s17, r0
    vmov    s18, r0
    vmov    s19, r0
    vmov    s20, r0
    vmov    s21, r0
    vmov    s22, r0
    vmov    s23, r0
    vmov    s24, r0
    vmov    s25, r0
    vmov    s26, r0
    vmov    s27, r0
    vmov    s28, r0
    vmov    s29, r0
    vmov    s30, r0
    vmov    s31, r0
    bx      lr
    .size kal_fpu_fill, . - kal_fpu_fill

    .global kal_fpu_read
    .thumb_func
    .type kal_fpu_read, %function
kal_fpu_read:
    vstmia  r0, {s16-s31}
    bx      lr
    .size kal_fpu_read, . - kal_fpu_read

    .global kal_fpu_set_fpscr
    .thumb_func
    .type kal_fpu_set_fpscr, %function
kal_fpu_set_fpscr:
    vmsr    fpscr, r0
    bx      lr
    .size kal_fpu_set_fpscr, . - kal_fpu_set_fpscr

    .global kal_fpu_fpscr
    .thumb_func
    .type kal_fpu_fpscr, %function
kal_fpu_fpscr:
    vmrs    r0, fpscr
    bx      lr
    .size kal_fpu_fpscr, . - kal_fpu_fpscr

    .global kal_cpu_control
    .thumb_func
    .type kal_cpu_control, %function
kal_cpu_control:
    mrs     r0, control
    bx      lr
    .size kal_cpu_control, . - kal_cpu_control

    .global kal_fpu_fpccr
    .thumb_func
    .type kal_fpu_fpccr, %function
kal_fpu_fpccr:
    ldr     r0, =0xE000EF34       /* FPCCR, registre architectural ARMv7-M */
    ldr     r0, [r0]
    bx      lr
    .size kal_fpu_fpccr, . - kal_fpu_fpccr
    .ltorg
