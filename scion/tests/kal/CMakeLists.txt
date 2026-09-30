# Banc KAL sous QEMU (BANC-TEST-KAL-QEMU.md) : T0 = smoke.lsh (fixture kal_t0), T1-T8 = kal_bench,
# plus T1F/T4F/T6F/T7F (contexte FPU actif) en ABI hard.
# Un exécutable par (architecture × backend) : même noyau et même édition de liens que lepton
# (lepton_link_firmware), main de test à la place du main du micro-noyau. Terminaison et code de
# retour par semihosting.
enable_language(ASM)

set(kal_isa_asm)
if(LEPTON_ISA STREQUAL "armv7m")
  set(kal_isa_asm kal_regs_armv7m.S kal_regs_read_armv7m.S)
  # variantes FPU (écart E3) : cœur à FPU compilé en ABI hard seulement
  if(LEPTON_FLOAT_ABI STREQUAL "hard")
    list(APPEND kal_isa_asm kal_fpu_armv7m.S)
    set(kal_fpu_tests T1F T4F T6F T7F)
  endif()
else()
  message(STATUS "banc KAL : pas de fichiers de registres pour LEPTON_ISA=${LEPTON_ISA}")
  return()
endif()

set(kal_platform_sources ${LEPTON_ISA_STARTUP_SOURCES} ${LEPTON_KAL_HW_SOURCES})
list(TRANSFORM kal_platform_sources PREPEND ${LEPTON_SRC}/)
add_executable(kal_bench kal_bench.c kal_semihost.c ${kal_isa_asm} ${kal_platform_sources})
target_include_directories(kal_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
lepton_link_firmware(kal_bench)

set(kal_qemu ${LEPTON_QEMU_ARM} -M ${LEPTON_QEMU_MACHINE} -kernel $<TARGET_FILE:kal_bench>
    -nographic -monitor none -serial null)
set(kal_labels "kal;arch:${LEPTON_CPU};backend:${LEPTON_KAL_BACKEND}")

foreach(t T1 T2 T3 T4 T5 T6 T7 T8 ${kal_fpu_tests} IRQ)
  add_test(NAME kal.${t}
           COMMAND ${kal_qemu} -semihosting-config enable=on,target=native,arg=kal_bench,arg=${t})
  set_tests_properties(kal.${t} PROPERTIES LABELS "${kal_labels}" TIMEOUT 20
                       FIXTURES_REQUIRED kal_t0)
endforeach()

# le harnais doit échouer correctement (code de retour QEMU non nul)
add_test(NAME kal.harness_fail
         COMMAND ${kal_qemu} -semihosting-config enable=on,target=native,arg=kal_bench,arg=HARNESS_FAIL)
set_tests_properties(kal.harness_fail PROPERTIES LABELS "${kal_labels}" TIMEOUT 20 WILL_FAIL TRUE)
