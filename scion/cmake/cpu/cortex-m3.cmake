# Cœur Cortex-M3 (étape 6 ; QEMU mps2-an385). Bibliothèque embOS libosT7L<mode>.a.
set(cpu_flags -mcpu=cortex-m3 -mfloat-abi=soft)
set(LEPTON_EMBOS_LIB_FAMILY T7L)
target_compile_options(lepton_options INTERFACE ${cpu_flags})
target_link_options(lepton_options INTERFACE ${cpu_flags})
# Nom du cœur (uname, __KERNEL_CPU_NAME de kernelconf.h).
target_compile_definitions(lepton_options INTERFACE __KERNEL_CPU_NAME="cortexm3")
