# mklepton_check.cmake — critère mklepton de l'étape 2 (reformulé le 2026-09-30, oracle sans binaire).
#   cmake -DMKLEPTON=<exe> -DUFS_CHECK=<exe> -DSRC=<trunk> -DXML=<mkconf relatif> -DTARGET=<cible>
#         -DWORK=<répertoire> -P mklepton_check.cmake
# 1. déterminisme : deux exécutions (SOURCE_DATE_EPOCH=0) → sorties identiques octet à octet ;
# 2. structure : conforme au jeu de référence (doc/migration/mklepton-ref.md) ;
# 3. image : signature « ufs 1.5 », relue par le noyau statique (pseudo-binaires du mkconf).

foreach(v MKLEPTON UFS_CHECK SRC XML TARGET WORK)
  if(NOT DEFINED ${v})
    message(FATAL_ERROR "variable ${v} manquante")
  endif()
endforeach()

file(REMOVE_RECURSE ${WORK})
foreach(run a b)
  file(MAKE_DIRECTORY ${WORK}/${run})
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E env SOURCE_DATE_EPOCH=0
            ${MKLEPTON} -s ${SRC} -o ${WORK}/${run} -t ${TARGET} ${XML}
    RESULT_VARIABLE rc OUTPUT_FILE ${WORK}/${run}.log ERROR_FILE ${WORK}/${run}.log)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "mklepton (${run}) : code ${rc}, voir ${WORK}/${run}.log")
  endif()
endforeach()

# --- 1. déterminisme ---------------------------------------------------------------------------------
set(expected kernel_mkconf.h dev_mkconf.c bin_mkconf.c dev_dskimg.c dev_dskimg.h .boot .mount .fsflash.o)
foreach(f IN LISTS expected)
  if(NOT EXISTS ${WORK}/a/${f})
    message(FATAL_ERROR "sortie absente : ${f}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files ${WORK}/a/${f} ${WORK}/b/${f}
                  RESULT_VARIABLE diff)
  if(NOT diff EQUAL 0)
    message(FATAL_ERROR "sortie non déterministe : ${f}")
  endif()
endforeach()
file(GLOB produced RELATIVE ${WORK}/a ${WORK}/a/* ${WORK}/a/.*)
list(REMOVE_ITEM produced ${expected})
if(produced)
  message(FATAL_ERROR "sorties inattendues dans le répertoire de sortie : ${produced}")
endif()

# --- 2. structure (formes des sorties versionnées : mklepton-ref.md) ---------------------------------
function(require file regex what)
  file(READ ${WORK}/a/${file} text)
  if(NOT text MATCHES "${regex}")
    message(FATAL_ERROR "${file} : ${what} absent")
  endif()
endfunction()
foreach(m __KERNEL_CPU_FREQ __KERNEL_HEAP_SIZE __KERNEL_PTHREAD_MAX __KERNEL_PROCESS_MAX
          MAX_OPEN_FILE OPEN_MAX __KERNEL_ENV_PATH)
  require(kernel_mkconf.h "#define ${m}" "#define ${m}")
endforeach()
require(kernel_mkconf.h "#include \"dev_dskimg.h\"" "include de dev_dskimg.h")
require(kernel_mkconf.h "#include \"sys/user/" "include de user_kernel_mkconf.h (include_absolute_path)")
require(dev_mkconf.c "pdev_map_t const dev_lst\\[\\]" "table dev_lst")
require(dev_mkconf.c "pdev_map_t const \\* *pdev_lst" "pdev_lst")
require(dev_mkconf.c "max_dev" "max_dev")
require(bin_mkconf.c "bin_t _bin_lst\\[\\]" "table _bin_lst")
require(bin_mkconf.c "bin_lst_size" "bin_lst_size")
require(dev_dskimg.c "filecpu_memory\\[\\]" "tableau filecpu_memory")
require(dev_dskimg.h "filecpu_memory" "déclaration filecpu_memory")
foreach(f kernel_mkconf.h dev_mkconf.c bin_mkconf.c dev_dskimg.c dev_dskimg.h .boot .mount)
  file(READ ${WORK}/a/${f} text)
  if(text MATCHES "[cC]:[/\\\\]tauon|/home/")
    message(FATAL_ERROR "${f} : chemin d'un ancien poste ou absolu")
  endif()
endforeach()

# --- 3. image ------------------------------------------------------------------------------------
file(READ ${WORK}/a/.fsflash.o sig LIMIT 7 HEX)
if(NOT sig STREQUAL "7566732031 2e35" AND NOT sig STREQUAL "75667320312e35")
  message(FATAL_ERROR ".fsflash.o : signature ${sig} au lieu de « ufs 1.5 » (75667320312e35)")
endif()
# pseudo-binaires attendus : <bin name> des blocs <binaries dest_path="…">
file(STRINGS ${SRC}/${XML} lines)
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
file(COPY ${WORK}/a/.fsflash.o DESTINATION ${WORK}/check)
execute_process(COMMAND ${UFS_CHECK} ${bins} WORKING_DIRECTORY ${WORK}/check
                RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
message("${out}")
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "relecture de l'image : échec")
endif()
list(LENGTH bins nbins)
message("mklepton_check : ${XML} [${TARGET}] déterministe, structure conforme, ${nbins} pseudo-binaires relus")
