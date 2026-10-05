# Handoff étape 6 → étape 7 (backend FreeRTOS)

État au 2026-10-05 : branche `migration/etape-6`, tous les modules faits (M7 QEMU,
STM32F746G-DISCO, SAMD21 Xplained Pro, NUCLEO-WL55JC1, puis CI, retrait IAR, code gelé). Clôture
de l'étape **soumise à la validation de l'utilisateur**. Handoffs des cartes :
`etape-6-m7-qemu.md`, `etape-6-f746.md`, `etape-6-samd21.md`, `etape-6-wl55.md`.

## Réponses aux prérequis de l'étape 7
- Étape 6 close sous réserve de validation ; socle QEMU (an386 hard/soft, an500) et cartes verts
  sous embOS ; `ci/run.sh` vert (8 presets, garde IAR, mémoire), mass_compile 414/414.
- FreeRTOS vendored dans l'arbre : `ucore/freeRTOS_8-0-0` et `ucore/freeRTOS_9-0-0` (paquets
  tiers, non LTS) ; `core-freertos` et `kal/backend/freertos` présents. **Version LTS à épingler :
  décision de début d'étape 7** (aucune LTS dans l'arbre).
- `cartographie-kal.md` : inchangée par l'étape 6 (le contrat `kal/contrat.h` n'a pas bougé ;
  ISA `armv6m` ajoutée sous `kal/arch/`).

## Décisions actées pendant le module final (2026-10-05)
- `ci/Dockerfile` écrit, **jamais construit ni exécuté** (aucun moteur de conteneur sur l'hôte) :
  CI validée en natif ; critère « reproductible localement » partiellement ouvert.
- Fichiers IAR des **paquets tiers conservés** (43 : CMSIS, CMSIS-5, FreeRTOS 8/9, uip) ; tout
  `sys/root/prj/` supprimé (iar, scons, vc-2010, config).
- Code gelé supprimé, y compris les annexes hors `perimetre.csv` (heuristique d'`audit_iar.py`)
  et les restes des paquets embOS IAR (`ucore/embOS*`).

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `ci/run.sh` | pipeline complet ; `LEPTON_MEM_ALERT` (défaut 90 %) |
| `ci/memoire.py` | relevé `--print-memory-usage` → `$LEPTON_BUILD/ci/memoire.csv` |
| `ci/Dockerfile` | image de référence (hypothèses à valider en tête) |
| `tools/migration/audit_iar.py --garde` | garde CI : 0 IAR-isme Lepton actif |
| `tools/migration/retrait_iar.py`, `doc/migration/retrait-iar.md` | retrait IAR rejouable, liste |
| `tools/migration/retrait_gele.py`, `doc/migration/code-gele.md` | retrait du gelé, inventaire |
| `doc/BUILDING.md`, `doc/Doxyfile` | procédure de build, table cœurs × cartes × statut, Doxygen |
| tags locaux `legacy-iar`, `legacy` | état avant retrait IAR, avant retrait du gelé |

## Écarts au plan et pièges découverts
- `perimetre.csv` ne recense que le code (cloc) : 356 fichiers annexes des cibles gelées lui
  échappaient (repris sur décision utilisateur, commit séparé).
- `dev_fb.c` (actif) incluait sans usage un en-tête de la simulation Linux : révélé par
  `mass_compile` après le retrait, inclusion retirée.
- `scion graft` ne retire pas les liens orphelins : après une suppression dans le clone,
  `scion ungraft && scion graft` (la liste greffée est conservée, ce n'est pas `graft-clean`).
- Les suppressions indexées par `git rm` partent avec le commit suivant : committer le script
  par chemin (`git commit -- <fichier>`) avant `--apply`. Erreur commise sur le retrait IAR,
  corrigée par réécriture des commits locaux non publiés (accord utilisateur 2026-10-05).
- Occupation mémoire maximale : CCM de la F439 69 % ; RAM SAMD21 42,5 %.

## Dette transmise
- Gardes `WIN32` résiduelles (reportées de l'étape 4) dans `core-segger/fork.c`,
  `kernel_compiler.h`, `kernel_pthread.h` : code mort depuis le retrait du gelé, à retirer par
  `transform_iar.py` (règle `garde-cible-gelee`).
- Conteneur CI à construire et exécuter (tests réseau : espaces de noms utilisateur).
- Le reste de la dette : `MIGRATION-STATUS.md`, « Blocages et dette ».

## Non transmis volontairement
- Détail des fichiers retirés : `retrait-iar.md`, `code-gele.md`, tags `legacy-iar`/`legacy`.
- Journaux CI : `$LEPTON_BUILD/ci_run_etape6_*.log`, `$LEPTON_BUILD/ci/` (hors git).
