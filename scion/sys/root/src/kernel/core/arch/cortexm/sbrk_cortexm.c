/*
 * Lepton — extension du tas newlib (_sbrk) pour Cortex-M, GCC. Licence : voir LICENSE (MPL 1.1).
 *
 * Écrit pour la migration GCC/Linux (étape 5, décision utilisateur 2026-10-03). Le _sbrk de
 * libnosys ne connaît aucune limite : sur la NUCLEO-F439ZI, le tas (piles des processus Lepton
 * par _sys_malloc, session de ftpd) débordait sur la pile principale puis hors de la SRAM
 * (BusFault). Ici, le tas va de __heap_start__ à __stack_limit__ (ld/common-cortexm.ld) ; au-delà,
 * _sbrk échoue et malloc rend NULL. Commun à toutes les cartes Cortex-M ; lié comme objet de
 * l'exécutable (LEPTON_ISA_STARTUP_SOURCES) pour remplacer celui de libnosys.
 * Pas de mise à jour d'errno : malloc de newlib traite l'échec de _sbrk seul.
 */
#include <stddef.h>

extern char __heap_start__;
extern char __stack_limit__;

void* _sbrk(ptrdiff_t incr);

void* _sbrk(ptrdiff_t incr){
   static char* brk = &__heap_start__;
   char* prev = brk;

   if(incr > &__stack_limit__ - brk || incr < &__heap_start__ - brk)
      return (void*)-1;
   brk += incr;
   return prev;
}
