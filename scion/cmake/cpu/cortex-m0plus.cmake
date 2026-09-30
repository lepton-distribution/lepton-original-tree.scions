# Cœur Cortex-M0+ (étape 6 ; carte M0+ à choisir, QEMU microbit = M0). Bibliothèque embOS
# libosT6L<mode>.a (pas de variante VFP en ARMv6-M).
set(cpu_flags -mcpu=cortex-m0plus -mfloat-abi=soft)
set(LEPTON_EMBOS_LIB_FAMILY T6L)
target_compile_options(lepton_options INTERFACE ${cpu_flags})
target_link_options(lepton_options INTERFACE ${cpu_flags})
