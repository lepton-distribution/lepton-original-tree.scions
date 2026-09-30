# lepton_target.cmake — fonctions communes de déclaration des bibliothèques Lepton.
#
# Les options de compilation communes (ISA, cœur, carte, micro-noyau) sont portées par la cible
# d'interface lepton_options, alimentée par les fichiers d'axe (cmake/isa, cpu, boards, kal).
# Aucun flag spécifique compilateur hors de cmake/.

add_library(lepton_options INTERFACE)
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC})

# lepton_freestanding([<options du compilateur pour -print-file-name>])
# Le code Lepton ne voit aucun en-tête de la libc du système (types en conflit : time_t, ino_t,
# ssize_t…) : seuls ceux du compilateur (stdint, stdarg, stddef) et les déclarations minimales
# kernel/core/include/libc (string, stdlib, ctype, limits, math ; ABI standard). La libc du
# système (glibc sur l'hôte, newlib-nano sur cible, décision 2026-09-30) fournit les
# implémentations à l'édition de liens.
set(LEPTON_LIBC_DECL_DIR ${LEPTON_SRC}/kernel/core/include/libc)
function(lepton_freestanding)
  execute_process(COMMAND ${CMAKE_C_COMPILER} ${ARGN} -print-file-name=include
                  OUTPUT_VARIABLE gcc_include OUTPUT_STRIP_TRAILING_WHITESPACE)
  target_compile_options(lepton_options INTERFACE
    -ffreestanding -nostdinc
    "SHELL:-isystem ${LEPTON_LIBC_DECL_DIR}"
    "SHELL:-isystem ${gcc_include}")
endfunction()

# lepton_add_library(<nom> SOURCES <fichiers relatifs à sys/root/src> [DEPENDS <cibles>])
# Bibliothèque statique d'un composant ; DEPENDS suit le graphe doc/migration/dependances.md.
function(lepton_add_library name)
  cmake_parse_arguments(ARG "" "" "SOURCES;DEPENDS" ${ARGN})
  list(TRANSFORM ARG_SOURCES PREPEND ${LEPTON_SRC}/)
  add_library(${name} STATIC ${ARG_SOURCES})
  # PRIVATE : les options du noyau (freestanding) ne se propagent pas aux consommateurs, tels que
  # mklepton, compilé avec les en-têtes de la glibc.
  target_link_libraries(${name} PRIVATE lepton_options PUBLIC ${ARG_DEPENDS})
endfunction()
