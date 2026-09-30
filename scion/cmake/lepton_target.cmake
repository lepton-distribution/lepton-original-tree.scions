# lepton_target.cmake — fonctions communes de déclaration des bibliothèques Lepton.
#
# Les options de compilation communes (ISA, cœur, carte, micro-noyau) sont portées par la cible
# d'interface lepton_options, alimentée par les fichiers d'axe (cmake/isa, cpu, boards, kal).
# Aucun flag spécifique compilateur hors de cmake/.

add_library(lepton_options INTERFACE)
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC})

# lepton_add_library(<nom> SOURCES <fichiers relatifs à sys/root/src> [DEPENDS <cibles>])
# Bibliothèque statique d'un composant ; DEPENDS suit le graphe doc/migration/dependances.md.
function(lepton_add_library name)
  cmake_parse_arguments(ARG "" "" "SOURCES;DEPENDS" ${ARGN})
  list(TRANSFORM ARG_SOURCES PREPEND ${LEPTON_SRC}/)
  add_library(${name} STATIC ${ARG_SOURCES})
  target_link_libraries(${name} PUBLIC lepton_options ${ARG_DEPENDS})
endfunction()
