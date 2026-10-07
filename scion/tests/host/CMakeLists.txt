# Tests hôte (label « host ») : noyau statique, VFS, UFS, mklepton.

# --- VFS + UFS sur le noyau statique ---------------------------------------------------------------
add_executable(test_vfs_ufs vfs_ufs.c)
target_link_libraries(test_vfs_ufs PRIVATE lepton_options lepton_kernel)
set(vfs_ufs_dir ${CMAKE_CURRENT_BINARY_DIR}/vfs_ufs)
file(MAKE_DIRECTORY ${vfs_ufs_dir})
add_test(NAME host.vfs_ufs COMMAND test_vfs_ufs WORKING_DIRECTORY ${vfs_ufs_dir})

# --- format UFS : assertions statiques côté hôte (compilation) et côté arm-none-eabi ---------------
add_library(test_ufs_format_host OBJECT ufs_format.c)
target_link_libraries(test_ufs_format_host PRIVATE lepton_options)
add_test(NAME host.ufs_format_host
         COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test_ufs_format_host)

find_program(LEPTON_ARM_GCC arm-none-eabi-gcc)
execute_process(COMMAND ${LEPTON_ARM_GCC} -print-file-name=include
                OUTPUT_VARIABLE arm_gcc_include OUTPUT_STRIP_TRAILING_WHITESPACE)
# Même configuration du noyau que l'hôte (seule l'ABI change) : AAPCS Cortex-M4, enums courts.
add_test(NAME host.ufs_format_arm
         COMMAND ${LEPTON_ARM_GCC} ${LEPTON_HOST_ARM_CHECK_FLAGS} -std=gnu99 -ffreestanding -nostdinc
                 -isystem ${LEPTON_LIBC_DECL_DIR} -isystem ${arm_gcc_include}
                 -DCPU_GNU32 -D__tauon_cpu_device__=__tauon_cpu_device_gnu_synthetic__
                 -DUSE_KERNEL_STATIC -I${LEPTON_SRC} -I${LEPTON_SRC}/${LEPTON_KAL_STATIC_DIR}
                 -I${LEPTON_SRC}/${LEPTON_KAL_ARCH_DIR} -I${LEPTON_SRC}/${LEPTON_KAL_BACKEND_DIR}
                 -fsyntax-only ${CMAKE_CURRENT_SOURCE_DIR}/ufs_format.c)

# --- concordance de kernel_stub.h (mklepton) avec les types du noyau ------------------------------
add_library(test_stub_check_kernel STATIC stub_check_kernel.c)
target_link_libraries(test_stub_check_kernel PRIVATE lepton_options)
add_executable(test_stub_check stub_check_main.c)
target_include_directories(test_stub_check PRIVATE ${LEPTON_TOOLS}/mklepton/src)
target_link_libraries(test_stub_check PRIVATE test_stub_check_kernel)
add_test(NAME host.kernel_stub COMMAND test_stub_check)

# --- mklepton : déterminisme, structure, relecture de l'image (critère reformulé 2026-09-30) -------
add_executable(test_ufs_check ufs_check.c)
target_link_libraries(test_ufs_check PRIVATE lepton_options lepton_kernel)
# Base de la NUCLEO-F439ZI (étape 1) : application tauon-basic Olimex STM32-P407, cible Cortex-M.
add_test(NAME host.mklepton
         COMMAND ${CMAKE_COMMAND}
                 -DMKLEPTON=$<TARGET_FILE:mklepton> -DUFS_CHECK=$<TARGET_FILE:test_ufs_check>
                 -DSRC=${CMAKE_SOURCE_DIR}
                 -DXML=sys/user/tauon-basic/etc/mkconf_tauon_basic_lwip_stm32f4-olimex-p407.xml
                 -DTARGET=cortexm_lepton -DWORK=${CMAKE_CURRENT_BINARY_DIR}/mklepton
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/mklepton_check.cmake)

# --- mklepton : mêmes octets que la référence -m32 (étape 8), quel que soit l'hôte ---------------
add_test(NAME host.mklepton_empreintes
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/mklepton_empreintes.sh
                 --mklepton $<TARGET_FILE:mklepton> --trunk ${CMAKE_SOURCE_DIR}
                 --check ${CMAKE_CURRENT_SOURCE_DIR}/mklepton_empreintes.sha256)

set_tests_properties(host.vfs_ufs host.ufs_format_host host.ufs_format_arm host.kernel_stub
                     host.mklepton host.mklepton_empreintes
                     PROPERTIES LABELS host)
