# Handoff étape 7 (backend FreeRTOS) → suite du chantier

État au 2026-10-06 : branche `migration/etape-7` (locale, non fusionnée), modules 7.1 (QEMU an386),
7.2 (NUCLEO-F439ZI, sur F429ZI) et 7.3 (autres cibles, CI) faits. Dernière étape du plan
séquentiel (0 → 7) : pas d'`ETAPE-8`. Handoffs de module : `etape-7-qemu.md`, `etape-7-f439.md`.

## Réponses aux critères de l'étape 7
- `kal-freertos-ecarts.md` complet : macros (§2), points tranchés (§3), accès internes I1/I2
  (§4), constats d'implémentation 7.1-7.2 (§5) et 7.3 (§6).
- Banc KAL identique sur les deux backends, test par test : an386 hard (18) et soft (14), an500
  (18), F429ZI, F746, WL55 et SAMD21 sur carte. Écart connu : T2 (PC de départ = point
  d'entrée sous FreeRTOS, trampoline sous embOS). **SAMD21 : système complet non supporté sous
  FreeRTOS** (tas insuffisant, écart accepté le 2026-10-06), KAL validé (banc 14/14).
- F439 validée en FreeRTOS (paliers 1-8, module 7.2) ; suite rejouée sur le binaire final après
  l'alignement de `kernelconf.h` (voir `validation-nucleo-f439zi.md`).
- « Zéro modification hors `kal/backend/freertos/` » : **écarts acceptés** le 2026-10-06 (liste
  ci-dessous).
- Matrice micro-noyau × machine : `ci/run.sh` (6 presets QEMU testés, 8 presets carte construits),
  vert.

## Décisions actées pendant l'étape 7
- FreeRTOS 202604 LTS (noyau V11.3.0) vendored ; reprise de `core-freertos` (pas de noyau commun).
- Flash interne autorisée par module ; endurances : F439 et F746 4 h, WL55 et SAMD21 1 h.
- Correctif du pilote commun `dev_cmsdk_uart` (7.2).
- `kernelconf.h` : extensions POSIX (signaux temps réel, verrous de fichiers) identiques sous les
  deux micro-noyaux (7.3).
- SAMD21 : écart accepté ; piles idle et temporisateurs réduites pour la carte.
- **Backend embOS maintenu** (décision ORCHESTRATION §4) : les deux backends restent de premier
  rang ; embOS est le seul backend complet sur la SAMD21.

## Écarts acceptés au critère « zéro modification hors `kal/backend/freertos/` »
`core-freertos/` ; branches FreeRTOS de `interrupt.h`, `rttimer.h`, `core_rttimer.h`, `kernel.h`,
`kernel_pthread.h` ; `kernelconf.h` ; `lwip/ports/freertos/` ; `dev_cmsdk_uart` ;
`ld/common-cortexm.ld` (`.ccm_bss`) ; `cmake/kal/freertos.cmake`, `cmake/cpu/*`,
`cmake/boards/{stm32f746g-disco,samd21-xplained-pro}.cmake`, `cmake/components/kernel.cmake` ;
presets ; `ci/run.sh` ; banc (`tests/kal/arch/armv7m/backend/<backend>/`) ; `debug/gdbinit-*`.
Aucune modification de `core-segger` ni de `kal/backend/embos`.

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `doc/migration/kal-freertos-ecarts.md` | écarts embOS/FreeRTOS, accès internes, constats par module |
| `doc/migration/validation-*.md` (sections « Étape 7 ») | paliers par carte sous FreeRTOS, empreintes |
| `kal/backend/freertos/` | `kal_backend.h`, `kal_freertos.c`, `FreeRTOSConfig.h`, `arch/{armv7m,armv6m}/kal_freertos_frame.h` |
| `cmake/kal/freertos.cmake` | port par cœur (`LEPTON_FREERTOS_PORT`), surcharges par carte (`LEPTON_FREERTOS_CONFIG`) |
| `tools/migration/freertos_rebase.py`, `freertos-rebase.md` | remise à niveau de `core-freertos` |
| `ci/run.sh` | matrice `kernels` × `qemu_machines` / `board_machines` |

## Empreinte FreeRTOS / embOS (artefacts CI, `-Os`)
| Cible | Code | RAM statique |
|---|---|---|
| an386, an500 | +2,4 % | +8,9 % |
| F439 | +2,3 % | +8,1 % (mémoire FreeRTOS en CCM) |
| F746 | +1,8 % | +7,1 % |
| WL55 | +4,7 % | +25 % |
| SAMD21 | +4,8 % | +32 % (+4,5 Ko : tas insuffisant) |

## Écarts au plan et pièges découverts
- Sémaphore FreeRTOS = file générique (80 o, 8 sous embOS), tâches idle et temporisateurs réelles,
  listes de prêts par priorité : coût fixe ≈ 4,5 Ko, rédhibitoire sur 32 Ko.
- Piles de `lsh`/`initd` plus chargées sous FreeRTOS (tas de thread +200 o, cause non analysée) :
  `lsh` 78 % sur F429/F746.
- `ARM_CM4F` refuse un M7 r0p1 (`configASSERT` sur le CPUID) ; `ARM_CM0` V11 : `portasm.c` à
  compiler, `configENABLE_MPU` obligatoire, trame à EXC_RETURN en tête.
- Le hook `lepton_guard.py` résout les chemins depuis le répertoire courant de la session et lit
  `>` dans les arguments comme une redirection : scripts dans le scratchpad, `git commit -F`.
- `ci/run.sh` efface les `.elf` des presets : ne pas le lancer pendant une endurance.

## Non transmis volontairement
- Journaux : `$LEPTON_BUILD/<preset>/endurance_*`, `$LEPTON_BUILD/ci_run_etape7_*.log`.
- Scripts de diagnostic gdb du scratchpad (relevé du tas `brk.0`, point d'arrêt sur `_sbrk`).
