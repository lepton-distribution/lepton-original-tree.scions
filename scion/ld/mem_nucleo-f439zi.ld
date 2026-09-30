/* Mémoire du STM32F439ZI (NUCLEO-F439ZI) : flash 2 Mo à 0x08000000, SRAM1+2+3 contiguës
 * 192 Ko à 0x20000000, CCM 64 Ko à 0x10000000 (données seulement, pas de DMA).
 * HYPOTHÈSE À VALIDER (étape 5, RM0090) : 192 Ko contiguës (le .ld du BSP embOS F429ZI n'en
 * déclare que 112 Ko). Validé à l'étape 5. Régions nommées pour ld/common-cortexm.ld. */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 192K
  CCM   (rw)  : ORIGIN = 0x10000000, LENGTH = 64K
}
REGION_ALIAS("REGION_TEXT", FLASH);
REGION_ALIAS("REGION_DATA", RAM);
