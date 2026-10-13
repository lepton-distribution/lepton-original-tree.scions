# mklepton_format_check.cmake — format XML 2 de mklepton (ENV-4, CFG-8).
#   cmake -DMKLEPTON=<exe> -DUFS_CHECK=<exe> -DSRC=<trunk> -DXML=<mkconf relatif> -DCPU_FREQ=<Hz>
#         -DKERNEL_CONFIG_HEADER=<en-tête relatif> -DWORK=<répertoire> -P mklepton_format_check.cmake
# Variantes dérivées du mkconf donné (format 2, <cpufs size node blocksz>) :
# 1. version inconnue, version absente, élément <target> du format 1 : refus (code non nul, message) ;
# 2. <cpufs> sans size ni node : image dimensionnée automatiquement, relue par le noyau statique ;
# 3. size ou node imposés trop petits : refus, minimum indiqué ; minimum exact : accepté.

foreach(v MKLEPTON UFS_CHECK SRC XML CPU_FREQ KERNEL_CONFIG_HEADER WORK)
  if(NOT DEFINED ${v})
    message(FATAL_ERROR "variable ${v} manquante")
  endif()
endforeach()

file(REMOVE_RECURSE ${WORK})
file(MAKE_DIRECTORY ${WORK})
file(READ ${SRC}/${XML} base)
if(NOT base MATCHES "<cpufs size=\"[0-9]+\" node=\"[0-9]+\" ")
  message(FATAL_ERROR "${XML} : <cpufs size=… node=…> attendu")
endif()
string(REGEX REPLACE "<cpufs size=\"[0-9]+\" node=\"[0-9]+\" " "<cpufs " auto "${base}")

# mklepton(<nom> <texte XML> <variable code> <variable sortie>)
function(run_mklepton name text rc_var out_var)
  file(MAKE_DIRECTORY ${WORK}/${name})
  file(WRITE ${WORK}/${name}.xml "${text}")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E env SOURCE_DATE_EPOCH=0
            ${MKLEPTON} -s ${SRC} -o ${WORK}/${name}
            --cpu-freq ${CPU_FREQ} --kernel-config-header ${KERNEL_CONFIG_HEADER} ${WORK}/${name}.xml
    WORKING_DIRECTORY ${WORK}/${name}
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
  file(WRITE ${WORK}/${name}.log "${out}")
  set(${rc_var} ${rc} PARENT_SCOPE)
  set(${out_var} "${out}" PARENT_SCOPE)
endfunction()

function(expect_refused name text regex)
  run_mklepton(${name} "${text}" rc out)
  if(rc EQUAL 0)
    message(FATAL_ERROR "${name} : accepté, refus attendu (voir ${WORK}/${name}.log)")
  endif()
  if(NOT out MATCHES "${regex}")
    message(FATAL_ERROR "${name} : message « ${regex} » absent (voir ${WORK}/${name}.log)")
  endif()
  message("${name} : refusé (code ${rc})")
endfunction()

# --- 1. version et éléments du format 1 --------------------------------------------------------
string(REPLACE "version=\"2\"" "version=\"3\"" v3 "${base}")
expect_refused(version_inconnue "${v3}" "unknown format version \"3\" \\(expected \"2\"\\)")
string(REPLACE " version=\"2\"" "" v1 "${base}")
expect_refused(version_absente "${v1}" "no format version")
string(REPLACE "<kernel>" "<target name=\"cortexm_lepton\"></target>\n<kernel>" t1 "${base}")
expect_refused(element_target "${t1}" "element <target> is not part of format 2")

# --- 2. dimensionnement automatique -------------------------------------------------------------
run_mklepton(auto "${auto}" rc out)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "auto : code ${rc} (voir ${WORK}/auto.log)")
endif()
if(NOT out MATCHES "cpufs: size=([0-9]+) node=([0-9]+)")
  message(FATAL_ERROR "auto : dimensions non affichées (voir ${WORK}/auto.log)")
endif()
set(size ${CMAKE_MATCH_1})
set(node ${CMAKE_MATCH_2})
file(SIZE ${WORK}/auto/.fsflash.o img)
if(NOT img EQUAL size)
  message(FATAL_ERROR "auto : image de ${img} octets, ${size} annoncés")
endif()
# pseudo-binaires attendus : <bin name> des blocs <binaries dest_path="…">
file(STRINGS ${WORK}/auto.xml lines)
set(bins)
set(dest)
foreach(l IN LISTS lines)
  if(l MATCHES "<binaries[^>]*dest_path=\"([^\"]+)\"")
    set(dest ${CMAKE_MATCH_1})
  elseif(l MATCHES "^[ \t]*<bin name=\"([^\"]+)\"" AND dest)
    list(APPEND bins /usr/${dest}/${CMAKE_MATCH_1})
  endif()
endforeach()
file(MAKE_DIRECTORY ${WORK}/check)
file(COPY ${WORK}/auto/.fsflash.o DESTINATION ${WORK}/check)
execute_process(COMMAND ${UFS_CHECK} ${bins} WORKING_DIRECTORY ${WORK}/check
                RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "auto : relecture de l'image en échec\n${out}")
endif()
message("auto : size=${size} node=${node}, image relue")

# --- 3. valeurs imposées ------------------------------------------------------------------------
math(EXPR size_moins "${size} - 1")
math(EXPR node_moins "${node} - 1")
string(REPLACE "<cpufs " "<cpufs size=\"${size_moins}\" node=\"${node}\" " t "${auto}")
expect_refused(size_trop_petit "${t}" "size=\"${size_moins}\"> too small \\(minimum: ${size}\\)")
string(REPLACE "<cpufs " "<cpufs size=\"${size}\" node=\"${node_moins}\" " t "${auto}")
expect_refused(node_trop_petit "${t}" "node=\"${node_moins}\"> too small \\(minimum: ${node}\\)")
string(REPLACE "<cpufs " "<cpufs size=\"${size}\" node=\"${node}\" " t "${auto}")
run_mklepton(minimum "${t}" rc out)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "minimum : size=${size} node=${node} refusés (voir ${WORK}/minimum.log)")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files ${WORK}/auto/.fsflash.o ${WORK}/minimum/.fsflash.o
                RESULT_VARIABLE diff)
if(NOT diff EQUAL 0)
  message(FATAL_ERROR "minimum : image différente de l'image automatique")
endif()
message("mklepton_format_check : ${XML} conforme")
