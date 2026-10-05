/* Mémoire du STM32F746NG (STM32F746G-DISCO) : flash 1 Mo à 0x08000000 (interface AXIM ; taille
 * relue par la sonde, 0x1FF0F442 = 1024 Ko), RAM 320 Ko contiguës à 0x20000000 : DTCM 64 Ko,
 * SRAM1 240 Ko, SRAM2 16 Ko (RM0385 §2.2.2). SDRAM externe de la carte (8 Mo, FMC) non utilisée.
 * Régions nommées pour ld/common-cortexm.ld. */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 320K
}
REGION_ALIAS("REGION_TEXT", FLASH);
REGION_ALIAS("REGION_DATA", RAM);
/* pas de CCM sur le F7 : .ccm_bss (ld/common-cortexm.ld) en RAM */
REGION_ALIAS("REGION_CCM", RAM);
