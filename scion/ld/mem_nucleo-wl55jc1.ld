/* Mémoire du STM32WL55JC (NUCLEO-WL55JC1), Cortex-M4 (CPU1) seul : flash 256 Ko à 0x08000000
 * (taille relue par la sonde, FLASHSIZE 0x0100), SRAM1 + SRAM2 contiguës 64 Ko à 0x20000000
 * (RM0453 ; .icf IAR stm32wl55xC_m4-lepton.icf : 0x20000000-0x2000FFFF). Régions nommées pour
 * ld/common-cortexm.ld. */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 256K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}
REGION_ALIAS("REGION_TEXT", FLASH);
REGION_ALIAS("REGION_DATA", RAM);
/* pas de CCM : .ccm_bss en RAM */
REGION_ALIAS("REGION_CCM", RAM);
/* pile principale (MSP : démarrage, interruptions) : 4 Ko au lieu de 8 Ko (64 Ko de RAM ; IAR :
   0x600), à ajuster sur mesure (lepton-stacks) */
__main_stack_size__ = 0x1000;
