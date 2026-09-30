# mklepton natif (outil hôte, guide §1.2) : lié au noyau statique (lepton_kernel) et à expat.
#
# mklepton.c est compilé avec les en-têtes de la glibc (pas lepton_options) : il ne voit le noyau
# qu'à travers tools/mklepton/src/kernel_stub.h, dont la concordance avec les vrais types est
# vérifiée par tests/host/kernel_stub_check.c.

add_executable(mklepton ${LEPTON_TOOLS}/mklepton/src/mklepton.c)
target_include_directories(mklepton PRIVATE ${LEPTON_TOOLS}/mklepton/src)
target_compile_options(mklepton PRIVATE -std=gnu99)
target_link_libraries(mklepton PRIVATE lepton_kernel expat)
