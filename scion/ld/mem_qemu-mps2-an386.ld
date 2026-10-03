/* Mémoire de la carte QEMU mps2-an386 (Cortex-M4). Relevé QEMU 8.2 (MIGRATION-STATUS) :
 * SSRAM1 à 0x00000000 et SSRAM2 à 0x20000000, 4 Mo chacune. Code et vecteurs en SSRAM1 (chargés
 * par QEMU, -kernel), données en SSRAM2. Régions nommées pour ld/common-cortexm.ld. */
MEMORY
{
  CODE (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
  RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}
REGION_ALIAS("REGION_TEXT", CODE);
REGION_ALIAS("REGION_DATA", RAM);
/* pas de CCM : .ccm_bss (ld/common-cortexm.ld) en RAM */
REGION_ALIAS("REGION_CCM", RAM);
