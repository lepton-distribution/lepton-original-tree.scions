# Handoff étape 7, module 7.1 (QEMU mps2-an386) → module 7.2 (NUCLEO-F439ZI)

État au 2026-10-06 : branche `migration/etape-7` (locale). Plan approuvé : 7.1 socle QEMU
an386 hard-float, 7.2 F439 (sur F429ZI), 7.3 autres cibles, CI micro-noyau × machine, clôture.

## Réponses aux prérequis de 7.2
- FreeRTOS **202604 LTS, noyau V11.3.0** vendored : `ucore/freeRTOS_11-3-0` (`LEPTON-PROVENANCE.md`,
  SHA-256 de l'archive) ; ports GCC CM0, CM3, CM4F, CM7/r0p1.
- Backend opérationnel sur `qemu-mps2-an386-freertos` (M4F, hard, port `ARM_CM4F`) : `smoke.lsh`,
  `net.ping_ftpd`, banc KAL T1-T8, T1F/T4F/T6F/T7F, TICI, TCLK, TSBRK, IRQ verts (18/18) ; preset
  dans `ci/run.sh` ; `ci/run.sh` vert (9 presets).
- Empreinte `lepton.elf` an386 (`memoire.csv`) : embOS code 281 448 / RAM 129 640 o ; FreeRTOS
  287 572 / 140 768 o (+2,2 % / +8,6 % : TCB et groupe d'événements statiques par pthread, piles
  idle et temporisateurs).
- F439 : même cœur et même port que l'an386 ; à faire : preset `nucleo-f439zi-freertos`, paliers 1-8
  de l'étape 5 (`validation-nucleo-f439zi.md`), `ctest -L board`. Flash : accord à chaque fois.

## Décisions actées pendant 7.1
- 2026-10-06 : V11.3.0 ; **reprise de `core-freertos`** (plutôt qu'un noyau commun).
- Écarts au critère « zéro modification hors `kal/backend/freertos/` » (à accepter en fin
  d'étape) : `core-freertos/` ; branches FreeRTOS de `interrupt.h`, `rttimer.h`,
  `core_rttimer.h` ; condition du verrou dans `kernel.h` ; `lwip/ports/freertos/` ;
  `cmake/kal/freertos.cmake`, `cmake/cpu/cortex-m4f.cmake` (`LEPTON_FREERTOS_PORT`),
  `cmake/components/kernel.cmake` (`LEPTON_LWIP_SYS_ARCH_DIR`), preset, `ci/run.sh` ; banc :
  services du micro-noyau extraits dans `tests/kal/arch/armv7m/backend/<backend>/`.
  **Aucune modification de `core-segger` ni de `kal/backend/embos`.**

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `doc/migration/kal-freertos-ecarts.md` | écarts macro par macro, accès internes I1/I2, constats de l'implémentation (§5) |
| `tools/migration/freertos_rebase.py`, `freertos_rebase.json`, `doc/migration/freertos-rebase.md` | remise à niveau de `core-freertos` (fusion 3 voies, classement bloc par bloc) |
| `kal/backend/freertos/` | `kal_backend.h`, `kal_freertos.c` (région atomique), `FreeRTOSConfig.h`, `arch/<isa>/kal_freertos_frame.h` |
| `core-freertos/arch/armv7m/` | `freertos_main.c`, `freertos_hooks.c` (cause d'arrêt dans `lepton_freertos_assert_file/line`, `lepton_freertos_overflow_task`) |
| `tests/kal/arch/armv7m/backend/{embos,freertos}/kal_bench_os.h` | services du micro-noyau et oracle de trame du banc |

## Écarts au plan et pièges découverts
- `core-freertos` est une lignée ancienne du noyau, pas un backend : ~1 200 lignes d'écart ; seuls
  48 blocs propres à FreeRTOS gardés. Défauts latents trouvés : `tmr_t` incohérent entre deux
  en-têtes (corruption), mutex binaire, temps restant des temporisateurs à 0, priorité figée à 4.
- Bloquer dans `__atomic_in` est légal sous embOS, pas sous FreeRTOS : `kal_freertos.c`.
- Diagnostic d'un arrêt FreeRTOS sous QEMU : `-gdb tcp::3333` puis lire
  `lepton_freertos_assert_file/line` (script de diagnostic : copie de `net_qemu.py` + gdb).
- `grep -r` (ugrep) ne suit pas les liens du trunk : `grep -R`.
- Le hook refuse une redirection `>` suivie d'un `"` (adresse du `Co-Authored-By`) : messages de
  commit par fichier (`git commit -F`).
- `transform_iar.py` recrée `scion/legacy/` (supprimé à l'étape 6) : copies retirées après coup.
- Banc embOS après extraction : code objet de `kal_bench.c` identique sur an386 (hard, soft) ;
  sur les cinq autres presets, `test_t2` (xPSR lu par l'oracle qui tient compte d'un cadre FPU)
  et `test_irq` (ticks en `uint32_t`, comparaison non signée) diffèrent, sans effet attendu ;
  QEMU embOS vert, cartes compilées seulement.
- Ne rien modifier dans l'arbre pendant `ci/run.sh` (une exécution invalidée ainsi).

## Non transmis volontairement
- Détail bloc par bloc de la fusion : `freertos-rebase.md`. Journaux CI : `$LEPTON_BUILD/ci_run_etape7_*.log`.
- Port ARMv6-M (trame `ARM_CM0`) : `kal/backend/freertos/arch/armv6m` contient un `#error`, module 7.3.
