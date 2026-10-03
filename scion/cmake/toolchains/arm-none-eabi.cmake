# Toolchain arm-none-eabi-gcc (cibles Cortex-M, étapes 3 à 7). Fichier CMAKE_TOOLCHAIN_FILE des
# presets croisés ; les flags de cœur sont dans cmake/cpu/<cœur>.cmake.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE arm-none-eabi-size)

# Niveau d'optimisation des cibles Cortex-M (décision 2026-10-02, étape 5 palier 9 : -Os pour
# tous les presets croisés, QEMU et carte), appliqué par cmake/isa/<isa>.cmake ; -g conservé
# (CMAKE_BUILD_TYPE Debug). Surchargeable (ex. -DLEPTON_OPT_LEVEL=-O0 pour le débogage).
set(LEPTON_OPT_LEVEL -Os CACHE STRING "Niveau d'optimisation des cibles Cortex-M (-Os, -O0, -Og, -O2)")

# $<LINK_GROUP:RESCAN,…> (cycle des bibliothèques du noyau) : prédéfini par CMake pour Linux,
# pas pour CMAKE_SYSTEM_NAME Generic ; GNU ld.
set(CMAKE_C_LINK_GROUP_USING_RESCAN "LINKER:--start-group" "LINKER:--end-group")
set(CMAKE_C_LINK_GROUP_USING_RESCAN_SUPPORTED TRUE)

# Pas d'exécutable hôte lors des tests de compilateur (pas de startup ni de script de liens).
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
