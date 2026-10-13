# mklepton.cmake — génération de la configuration et de l'image par mklepton (guide §1.2).
#
# lepton_generate(<nom> XML <mkconf relatif au trunk> [CPU_FREQ <Hz>]
#                 [KERNEL_CONFIG_HEADER <en-tête relatif au trunk>] [MKLEPTON <exécutable>])
#
# Sorties dans ${LEPTON_GENERATED_DIR}/<nom>/ (décision 2026-09-30 : option -o de mklepton, les
# mkconf au format 2 n'ont plus de dest_path ; aucune écriture dans le trunk) : kernel_mkconf.h,
# dev_mkconf.c, bin_mkconf.c, dev_dskimg.c/.h, .boot, .mount, et l'image .fsflash.o.
# Chemins d'entrée des mkconf relatifs au trunk (-s). SOURCE_DATE_EPOCH fixe : sorties
# reproductibles. Crée la cible <nom>_mkconf et la variable <nom>_GENERATED_DIR.
# CPU_FREQ et KERNEL_CONFIG_HEADER : propriétés de la carte (ENV-4, PA-4), passées à mklepton par
# --cpu-freq et --kernel-config-header (__KERNEL_CPU_FREQ, #include de kernel_mkconf.h).
#
# MKLEPTON : exécutable hôte ; par défaut la cible « mklepton » du même build (preset host),
# sinon ${LEPTON_MKLEPTON} fourni par le superbuild des presets croisés (étape 3).

set(LEPTON_SOURCE_DATE_EPOCH 0 CACHE STRING "Horodatage des images UFS générées (SOURCE_DATE_EPOCH)")

function(lepton_generate name)
  cmake_parse_arguments(ARG "" "XML;CPU_FREQ;KERNEL_CONFIG_HEADER;MKLEPTON" "" ${ARGN})
  if(NOT ARG_XML)
    message(FATAL_ERROR "lepton_generate(${name}) : XML obligatoire")
  endif()
  if(ARG_MKLEPTON)
    set(tool ${ARG_MKLEPTON})
  elseif(TARGET mklepton)
    set(tool $<TARGET_FILE:mklepton>)
  elseif(LEPTON_MKLEPTON)
    set(tool ${LEPTON_MKLEPTON})
  else()
    message(FATAL_ERROR "lepton_generate(${name}) : aucun exécutable mklepton (LEPTON_MKLEPTON)")
  endif()

  set(out ${LEPTON_GENERATED_DIR}/${name})
  set(xml ${CMAKE_SOURCE_DIR}/${ARG_XML})
  set(board_opts)
  if(ARG_CPU_FREQ)
    list(APPEND board_opts --cpu-freq ${ARG_CPU_FREQ})
  endif()
  if(ARG_KERNEL_CONFIG_HEADER)
    list(APPEND board_opts --kernel-config-header ${ARG_KERNEL_CONFIG_HEADER})
  endif()

  # Contenu du rootfs référencé par le mkconf (src_file, src_path) : dépendances du générateur.
  file(READ ${xml} xml_text)
  string(REGEX MATCHALL "src_(file|path)=\"[^\"]+\"" refs "${xml_text}")
  set(deps ${xml})
  foreach(ref IN LISTS refs)
    string(REGEX REPLACE "^src_(file|path)=\"([^\"]+)\"$" "\\2" path "${ref}")
    if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${path})
      file(GLOB_RECURSE dir_files CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/${path}/*)
      list(APPEND deps ${dir_files})
    elseif(EXISTS ${CMAKE_SOURCE_DIR}/${path})
      list(APPEND deps ${CMAKE_SOURCE_DIR}/${path})
    endif()
  endforeach()
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${xml})

  set(outputs ${out}/kernel_mkconf.h ${out}/dev_mkconf.c ${out}/bin_mkconf.c
              ${out}/dev_dskimg.c ${out}/dev_dskimg.h)
  add_custom_command(
    OUTPUT ${outputs}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${out}
    COMMAND ${CMAKE_COMMAND} -E env SOURCE_DATE_EPOCH=${LEPTON_SOURCE_DATE_EPOCH}
            ${tool} -s ${CMAKE_SOURCE_DIR} -o ${out} ${board_opts} ${ARG_XML}
    DEPENDS ${deps} ${tool}
    WORKING_DIRECTORY ${out}
    COMMENT "mklepton ${ARG_XML} -> ${out}"
    VERBATIM)
  add_custom_target(${name}_mkconf DEPENDS ${outputs})
  set(${name}_GENERATED_DIR ${out} PARENT_SCOPE)
endfunction()
