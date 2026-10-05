/* Mémoire du ATSAMD21J18A (SAMD21 Xplained Pro) : flash 256 Ko à 0x00000000, SRAM 32 Ko à
 * 0x20000000 (DFP Microchip SAMD21 3.8.270 : FLASH_SIZE, HMCRAMC0_SIZE ; DID 0x10010100 relu
 * par la sonde le 2026-10-05). Régions nommées pour ld/common-cortexm.ld. */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 256K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
}
REGION_ALIAS("REGION_TEXT", FLASH);
REGION_ALIAS("REGION_DATA", RAM);
/* pas de CCM : .ccm_bss (ld/common-cortexm.ld) en RAM */
REGION_ALIAS("REGION_CCM", RAM);
/* pile principale (MSP : démarrage, _start_kernel avant OS_Start, interruptions) réduite à la
 * RAM de 32 Ko (8 Ko par défaut) ; mesurée au palier 8 (lepton-stacks). */
__main_stack_size__ = 0x600;
