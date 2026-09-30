/* Mémoire de la carte QEMU mps2-an386 (Cortex-M4). Relevé QEMU 8.2 (MIGRATION-STATUS) :
 * SSRAM1 à 0x00000000 et SSRAM2 à 0x20000000, 4 Mo chacune. Code en SSRAM1 (chargé par QEMU),
 * données en SSRAM2. Décrit à l'étape 2, validé à l'étape 3. */
MEMORY
{
  CODE (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
  RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}
