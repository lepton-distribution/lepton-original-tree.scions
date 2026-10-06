# Provenance (migration Lepton, étape 7)

FreeRTOS **202604 LTS**, noyau **FreeRTOS-Kernel V11.3.0** (archive du tag `V11.3.0`,
https://github.com/FreeRTOS/FreeRTOS-Kernel/archive/refs/tags/V11.3.0.tar.gz, téléchargée le
2026-10-06, SHA-256 `76530a6bab55233e34e07c8df59f0d4c2e06db473763f8d33277d4fa10950084`, calculé
au téléchargement : GitHub ne publie pas d'empreinte pour les archives de tag). Licence MIT
(`LICENSE.md`). Décision utilisateur du 2026-10-06.

Fichiers non modifiés. Sous-ensemble repris : sources du noyau (`*.c` de la racine),
`include/`, `portable/GCC/ARM_CM0` (ARMv6-M), `ARM_CM3` (ARMv7-M sans FPU), `ARM_CM4F` (ARMv7-M
avec FPU, M4F et M7 hors r0p1), `ARM_CM7/r0p1` (erratum 837070), `portable/MemMang/heap_*.c`,
`History.txt`, `README.md`, `manifest.yml`. Ne sont pas repris : les autres ports et
compilateurs, `examples/`, les fichiers CMake amont et le SBOM SPDX (récupérable dans l'archive).
Ajouter un port depuis la même archive si un cœur l'exige.

Les copies `freeRTOS_8-0-0` et `freeRTOS_9-0-0` restent présentes (code d'origine) ; seule
cette version est utilisée par `cmake/kal/freertos.cmake`.
