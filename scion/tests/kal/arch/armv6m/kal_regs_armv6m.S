/*
 * Banc KAL — maintien de motifs dans les registres sauvegardés par le micro-noyau (ARMv6-M).
 *
 * void kal_regs_hold(const uint32_t pattern[8], uint32_t out[8], void (*wait)(void));
 *   charge R4-R11 avec pattern[0..7], appelle wait() (la tâche s'y bloque : le micro-noyau
 *   sauvegarde R4-R11 dans le cadre de la pile de la tâche), puis écrit R4-R11 dans out[0..7].
 *   wait() respecte l'AAPCS (R4-R11 préservés) : toute différence vient d'une manipulation du
 *   contexte sauvegardé (KAL) pendant l'attente.
 * Thumb-1 (étape 6) : même contrat que kal_regs_armv7m.S ; LDM/STM/PUSH/POP ne portent que sur
 * R0-R7, R8-R11 transitent par des registres bas (MOV).
 */
    .syntax unified
    .thumb
    .text

    .global kal_regs_hold
    .thumb_func
    .type kal_regs_hold, %function
kal_regs_hold:
    push    {r4-r7, lr}
    mov     r3, r8
    mov     r4, r9
    mov     r5, r10
    mov     r6, r11
    push    {r3-r6}               /* R8-R11 de l'appelant */
    push    {r1, r2}              /* out, wait */
    sub     sp, sp, #4            /* alignement 8 octets de la pile (AAPCS) : 20 + 16 + 8 + 4 */
    ldr     r3, [r0, #16]
    mov     r8, r3
    ldr     r3, [r0, #20]
    mov     r9, r3
    ldr     r3, [r0, #24]
    mov     r10, r3
    ldr     r3, [r0, #28]
    mov     r11, r3
    ldmia   r0!, {r4-r7}
    blx     r2
    ldr     r0, [sp, #4]          /* out */
    stmia   r0!, {r4-r7}
    mov     r1, r8
    str     r1, [r0, #0]
    mov     r1, r9
    str     r1, [r0, #4]
    mov     r1, r10
    str     r1, [r0, #8]
    mov     r1, r11
    str     r1, [r0, #12]
    add     sp, sp, #12
    pop     {r3-r6}
    mov     r8, r3
    mov     r9, r4
    mov     r10, r5
    mov     r11, r6
    pop     {r4-r7, pc}
    .size kal_regs_hold, . - kal_regs_hold
