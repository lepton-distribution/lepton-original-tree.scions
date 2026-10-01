/*
 * Banc KAL — maintien de motifs dans les registres sauvegardés par le micro-noyau (ARMv7-M).
 *
 * void kal_regs_hold(const uint32_t pattern[8], uint32_t out[8], void (*wait)(void));
 *   charge R4-R11 avec pattern[0..7], appelle wait() (la tâche s'y bloque : le micro-noyau
 *   sauvegarde R4-R11 dans le cadre de la pile de la tâche), puis écrit R4-R11 dans out[0..7].
 *   wait() respecte l'AAPCS (R4-R11 préservés) : toute différence vient d'une manipulation du
 *   contexte sauvegardé (KAL) pendant l'attente.
 */
    .syntax unified
    .thumb
    .text

    .global kal_regs_hold
    .thumb_func
    .type kal_regs_hold, %function
kal_regs_hold:
    push    {r4-r11, lr}
    push    {r1}                  /* out, relu après wait() */
    sub     sp, sp, #4            /* alignement 8 octets de la pile (AAPCS) */
    mov     r12, r2               /* wait */
    ldmia   r0, {r4-r11}
    blx     r12
    add     sp, sp, #4
    pop     {r0}
    stmia   r0, {r4-r11}
    pop     {r4-r11, pc}
    .size kal_regs_hold, . - kal_regs_hold
