# Banc KAL sous QEMU (BANC-TEST-KAL-QEMU.md) et sur carte réelle par OpenOCD (étape 5) : T0 = test
# de fumée (fixture kal_t0, board_t0), T1-T8 = kal_bench,
# plus T1F/T4F/T6F/T7F (contexte FPU actif) en ABI hard.
# Un exécutable par (architecture × backend) : même noyau et même édition de liens que lepton
# (lepton_link_firmware), main de test à la place du main du micro-noyau. Terminaison et code de
# retour par semihosting. Sources propres à l'ISA (cadres de pile, registres, variantes de cœur
# FPU) sous arch/<isa>/.
enable_language(ASM)

set(kal_isa_asm)
if(LEPTON_ISA STREQUAL "armv7m")
  set(kal_isa_dir ${CMAKE_CURRENT_SOURCE_DIR}/arch/${LEPTON_ISA})
  set(kal_isa_asm ${kal_isa_dir}/kal_regs_armv7m.S ${kal_isa_dir}/kal_regs_read_armv7m.S)
  # variantes FPU (écart E3) : cœur à FPU compilé en ABI hard seulement
  if(LEPTON_FLOAT_ABI STREQUAL "hard")
    list(APPEND kal_isa_asm ${kal_isa_dir}/kal_fpu_armv7m.S)
    set(kal_fpu_tests T1F T4F T6F T7F)
  endif()
else()
  message(STATUS "banc KAL : pas de fichiers de registres pour LEPTON_ISA=${LEPTON_ISA}")
  return()
endif()

set(kal_platform_sources ${LEPTON_ISA_STARTUP_SOURCES} ${LEPTON_KAL_HW_SOURCES})
list(TRANSFORM kal_platform_sources PREPEND ${LEPTON_SRC}/)
add_executable(kal_bench ${kal_isa_dir}/kal_bench.c kal_semihost.c ${kal_isa_asm} ${kal_platform_sources})
target_include_directories(kal_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
lepton_link_firmware(kal_bench)

set(kal_labels "kal;arch:${LEPTON_CPU};backend:${LEPTON_KAL_BACKEND}")
set(kal_tests T1 T2 T3 T4 T5 T6 T7 T8 ${kal_fpu_tests} IRQ)

if(LEPTON_QEMU_MACHINE)
  set(kal_run ${LEPTON_QEMU_ARM} -M ${LEPTON_QEMU_MACHINE} -kernel $<TARGET_FILE:kal_bench>
      -nographic -monitor none -serial null -semihosting-config enable=on,target=native,arg=kal_bench,arg=)
  set(kal_fixture kal_t0)
  set(kal_timeout 20)
else()
  # carte réelle (étape 5) : semihosting par OpenOCD (tests/kal_openocd.py) ; kal_bench est flashé
  # une fois (fixture board_kal_flash), après la fumée qui flashe lepton (board_t0)
  string(APPEND kal_labels ";board")
  set(kal_openocd ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/kal_openocd.py
      --openocd ${LEPTON_OPENOCD} --cfg ${LEPTON_BOARD_OPENOCD_CFG})
  add_test(NAME kal.board_flash COMMAND ${kal_openocd} --program $<TARGET_FILE:kal_bench>)
  set_tests_properties(kal.board_flash PROPERTIES LABELS "${kal_labels}" TIMEOUT 120 RUN_SERIAL TRUE
                       FIXTURES_REQUIRED board_t0 FIXTURES_SETUP board_kal_flash)
  set(kal_run ${kal_openocd} --test=)
  set(kal_fixture board_kal_flash)
  set(kal_timeout 60)
endif()

foreach(t ${kal_tests})
  add_test(NAME kal.${t} COMMAND ${kal_run}${t})
  set_tests_properties(kal.${t} PROPERTIES LABELS "${kal_labels}" TIMEOUT ${kal_timeout}
                       RUN_SERIAL TRUE FIXTURES_REQUIRED ${kal_fixture})
endforeach()

# le harnais doit échouer correctement (code de retour non nul)
add_test(NAME kal.harness_fail COMMAND ${kal_run}HARNESS_FAIL)
set_tests_properties(kal.harness_fail PROPERTIES LABELS "${kal_labels}" TIMEOUT ${kal_timeout}
                     RUN_SERIAL TRUE WILL_FAIL TRUE)
if(NOT LEPTON_QEMU_MACHINE)
  set_tests_properties(kal.harness_fail PROPERTIES FIXTURES_REQUIRED board_kal_flash)
endif()
