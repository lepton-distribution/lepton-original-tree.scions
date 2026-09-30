# Firmware d'une carte (presets croisés) : configuration générée par mklepton, pseudo-binaires
# (sbin, bin) déclarés par le mkconf de la carte, exécutable lepton.elf et image lepton.bin.

if(NOT LEPTON_BOARD_MKCONF)
  message(STATUS "firmware : aucun mkconf pour la carte ${LEPTON_BOARD} (firmware non construit)")
  return()
endif()

# --- configuration générée (kernel_mkconf.h, dev_mkconf.c, bin_mkconf.c, dev_dskimg.c) ----------
lepton_generate(board XML ${LEPTON_BOARD_MKCONF} MKCONF_TARGET ${LEPTON_BOARD_MKCONF_TARGET})
# kernel_mkconf.h (chemin d'inclusion, kernelconf.h) et en-tête projet relatif au trunk
target_include_directories(lepton_options INTERFACE ${board_GENERATED_DIR} ${CMAKE_SOURCE_DIR})
foreach(lib IN LISTS LEPTON_KERNEL_GROUP)
  add_dependencies(${lib} board_mkconf)
endforeach()

add_library(lepton_generated STATIC
  ${board_GENERATED_DIR}/dev_mkconf.c
  ${board_GENERATED_DIR}/bin_mkconf.c
  ${board_GENERATED_DIR}/dev_dskimg.c)
target_link_libraries(lepton_generated PRIVATE lepton_options)
add_dependencies(lepton_generated board_mkconf)

# --- pseudo-binaires : <binaries src_path="…"><bin name="…"> du mkconf → src/<src_path>/<nom>.c ----
file(STRINGS ${CMAKE_SOURCE_DIR}/${LEPTON_BOARD_MKCONF} mkconf_lines)
set(sbin_sources)
set(bin_sources)
set(src_path)
foreach(l IN LISTS mkconf_lines)
  if(l MATCHES "<binaries[^>]*src_path=\"([^\"]+)\"")
    set(src_path ${CMAKE_MATCH_1})
  elseif(l MATCHES "^[ \t]*<bin name=\"([^\"]+)\"" AND src_path)
    set(bin_name ${CMAKE_MATCH_1})
    if(src_path MATCHES "^sbin")
      list(APPEND sbin_sources ${src_path}/${bin_name}.c)
    else()
      list(APPEND bin_sources ${src_path}/${bin_name}.c)
    endif()
  endif()
endforeach()
set(firmware_libs lepton_generated)
if(sbin_sources)
  lepton_add_library(lepton_sbin SOURCES ${sbin_sources})
  add_dependencies(lepton_sbin board_mkconf)
  list(APPEND firmware_libs lepton_sbin)
endif()
if(bin_sources)
  lepton_add_library(lepton_bin SOURCES ${bin_sources})
  add_dependencies(lepton_bin board_mkconf)
  list(APPEND firmware_libs lepton_bin)
endif()

# --- exécutables -----------------------------------------------------------------------------------
list(JOIN firmware_libs "," LEPTON_FIRMWARE_GROUP_CSV)

# lepton_link_firmware(<cible>) : édition de liens d'un exécutable de la carte (noyau complet,
# configuration générée, pseudo-binaires, micro-noyau, scripts de liens) ; utilisé par lepton et
# par le banc KAL (tests/kal).
function(lepton_link_firmware target)
  set_target_properties(${target} PROPERTIES SUFFIX .elf)
  add_dependencies(${target} board_mkconf)
  target_link_libraries(${target} PRIVATE lepton_options
    "$<LINK_GROUP:RESCAN,${LEPTON_FIRMWARE_GROUP_CSV},${LEPTON_KERNEL_GROUP_CSV}>"
    ${LEPTON_KAL_LINK_LIBS} ${LEPTON_SYSTEM_LIBS})
  target_link_options(${target} PRIVATE
    -T${LEPTON_BOARD_MEMORY_LD} -T${CMAKE_SOURCE_DIR}/ld/common-cortexm.ld
    -Wl,-Map=${CMAKE_BINARY_DIR}/${target}.map -Wl,--print-memory-usage)
  set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS
    ${LEPTON_BOARD_MEMORY_LD} ${CMAKE_SOURCE_DIR}/ld/common-cortexm.ld)
endfunction()

set(LEPTON_FIRMWARE_ISA_SOURCES ${LEPTON_FIRMWARE_SOURCES})
list(TRANSFORM LEPTON_FIRMWARE_SOURCES PREPEND ${LEPTON_SRC}/)
add_executable(lepton ${LEPTON_FIRMWARE_SOURCES})
lepton_link_firmware(lepton)
add_custom_command(TARGET lepton POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:lepton> ${CMAKE_BINARY_DIR}/lepton.bin
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:lepton>
  VERBATIM)

# --- test de fumée canonique (label smoke) : démarrage → lsh → uname -a, ls, ps, second port ------
if(LEPTON_QEMU_MACHINE)
  find_program(LEPTON_QEMU_ARM qemu-system-arm REQUIRED)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  add_test(NAME smoke.lsh
           COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/smoke_lsh.py
                   --qemu ${LEPTON_QEMU_ARM} --machine ${LEPTON_QEMU_MACHINE}
                   --kernel $<TARGET_FILE:lepton> --expect-machine ${LEPTON_BOARD_UNAME_MACHINE}
                   --command ls --command ps --uart1
                   --log ${CMAKE_BINARY_DIR}/smoke_lsh_uart0.log)
  # T0 du banc KAL = fumée canonique ; T1-T8 n'ont de sens que si T0 est vert (fixture)
  set_tests_properties(smoke.lsh PROPERTIES
                       LABELS "smoke;kal;arch:${LEPTON_CPU};backend:${LEPTON_KAL_BACKEND}"
                       TIMEOUT 120 FIXTURES_SETUP kal_t0)
  add_subdirectory(${CMAKE_SOURCE_DIR}/tests/kal ${CMAKE_BINARY_DIR}/tests/kal)
endif()
