/* Lepton — script de liens commun Cortex-M (GCC). Les régions REGION_TEXT et REGION_DATA sont
 * définies par ld/mem_<carte>.ld (passé avant ce fichier). Symboles utilisés par le démarrage
 * (kernel/core/arch/cortexm/startup_armv7m.c) et par newlib (end, pour _sbrk). */

ENTRY(Reset_Handler)

/* Pile principale (MSP : démarrage, interruptions, embOS avant OS_Start). */
__main_stack_size__ = DEFINED(__main_stack_size__) ? __main_stack_size__ : 0x2000;

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    /* suite de la table propre au MCU (IRQ 64 et au-delà, fournie par le BSP), contiguë */
    KEEP(*(.isr_vector_ext))
    . = ALIGN(4);
  } > REGION_TEXT

  .text :
  {
    *(.text .text.*)
    *(.rodata .rodata.*)
    KEEP(*(.init))
    KEEP(*(.fini))
    . = ALIGN(4);
  } > REGION_TEXT

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > REGION_TEXT
  .ARM.exidx :
  {
    __exidx_start = .;
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    __exidx_end = .;
  } > REGION_TEXT

  .preinit_array :
  {
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } > REGION_TEXT
  .init_array :
  {
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } > REGION_TEXT
  .fini_array :
  {
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } > REGION_TEXT

  /* Données du CPU seul, sans accès DMA (étape 5, décision utilisateur 2026-10-03) : en CCM
   * sur les STM32F4 (REGION_CCM, ld/mem_<carte>.ld), en RAM ailleurs. Liste blanche par nom de
   * section (-fdata-sections) : pools lwIP sauf PBUF_POOL (73 Ko, plus grand que la CCM ; le
   * pilote Ethernet copie entre ses tampons DMA Tx_Buff/Rx_Buff, restés en .bss, et lwIP), tables
   * et piles statiques du noyau. Placée avant .bss : la première règle qui retient une section
   * l'emporte. Mise à zéro par le démarrage (__ccm_bss_start__ .. __ccm_bss_end__). */
  .ccm_bss (NOLOAD) :
  {
    . = ALIGN(8);
    __ccm_bss_start__ = .;
    *(.bss.memp_memory_[A-O]*)
    *(.bss.memp_memory_PBUF_base)
    *(.bss.memp_memory_[Q-Z]*)
    *(.bss.opipe_lst .bss.ofile_lst .bss.flock_lst)
    *(.bss.rtfsinode_lst .bss.rtfsblk_lst .bss.rtfs_superblk)
    *(.bss.kernel_stack .bss.kernel_thread .bss.lwip_core_stack .bss.lwip_core_thread)
    . = ALIGN(8);
    __ccm_bss_end__ = .;
  } > REGION_CCM

  __data_load__ = LOADADDR(.data);
  .data :
  {
    . = ALIGN(4);
    __data_start__ = .;
    /* fonctions en RAM (__lepton_ramfunc) : copiées avec .data par le démarrage */
    *(.ramfunc .ramfunc.*)
    *(.data .data.*)
    . = ALIGN(4);
    __data_end__ = .;
  } > REGION_DATA AT> REGION_TEXT

  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
  } > REGION_DATA

  /* Non initialisée par le démarrage (conservée à travers un reset logiciel). */
  .noinit (NOLOAD) :
  {
    . = ALIGN(4);
    *(.noinit .noinit.*)
    . = ALIGN(8);
  } > REGION_DATA

  /* Tas newlib (_sbrk) : de « end » jusqu'à la pile principale. */
  .heap (NOLOAD) :
  {
    . = ALIGN(8);
    PROVIDE(end = .);
    PROVIDE(_end = .);
    __heap_start__ = .;
  } > REGION_DATA

  __stack_top__ = ORIGIN(REGION_DATA) + LENGTH(REGION_DATA);
  __stack_limit__ = __stack_top__ - __main_stack_size__;
  /* pile système (MSP) selon embOS (OS_STACK_GetSysStackBase/Size) */
  __stack_start__ = __stack_limit__;
  __stack_end__ = __stack_top__;
  ASSERT(__heap_start__ <= __stack_limit__, "RAM : données et pile principale se chevauchent")
}
