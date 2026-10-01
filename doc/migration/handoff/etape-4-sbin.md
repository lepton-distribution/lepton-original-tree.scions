# Handoff étape 4, module `sbin` → modules suivants

État au 2026-10-01 : module fait sur `migration/etape-4-sbin`, validé par l'utilisateur
le 2026-10-01 et fusionné. Module suivant : `bin`. Procédure générale : `handoff/etape-4-outillage.md`.

## Résultat du module
- Périmètre : 35 fichiers actifs (`sys/root/src/sbin`, dont `net/ifconfig.c`, `net/slipd.c`).
- `transform_iar.py --rule garde-cible-gelee` : `ps.c` (branches eCos, copie sous
  `scion/legacy/sys/root/src/sbin/`), `xmodem.c` (condition simplifiée en `defined(__UNIX__)`,
  aucune ligne de code retirée donc pas de copie : comportement prévu de l'outil).
- `axes_sbin.py gelee` (simulation Linux `CPU_GNU32`, extension D3a) : `lsh.c` (branche retirée,
  copie `legacy/`), `initd.c` (`CPU_GNU32` retiré de la condition ; `EVAL_BOARD` conservé).
- `stty.c` : `#include <ctype.h>` (seul échec de compilation : `toupper` non déclaré).
- `gcc -E -P` identique avant/après les commits mécaniques : hard, soft, mass_compile `sbin`.
- `mass_compile.sh` : `sbin` **35/35** ; périmètre 341/348 (`bin` 3/8, `tauon-basic` 7/9).
- `audit_isa_ifdef.py` : total 7 → **3** (tous dans `tauon-basic`). `audit_iar.py` : actif 92 inchangé.
- `ci/run.sh` vert (outils 47 tests, hôte 5/5, smoke, net, KAL hard 15/15 et soft 11/11).

## Décisions actées pendant le module (2026-10-01)
- Option A : `toupper` résolu par l'inclusion de `<ctype.h>` dans `stty.c` (comme `compress.c`,
  `ftpd`) ; `lib/libc/ctype/ctype.h` non modifié (option B écartée : touche `lib` validé).

## Artefacts produits (manifeste)
| Fichier | Contenu, quand le lire |
|---|---|
| `tools/migration/axes_sbin.py` | règle `gelee` de `sbin` (non rejouable sur l'état final : refus « rien à faire ») |
| `tools/migration/transform_iar.py` | `prototype-static` : préfixe restreint aux jetons de type (+ 2 tests) |
| `scion/legacy/sys/root/src/sbin/{ps,lsh}.c` | copies d'origine, gelées, supprimées à l'étape 6 |

## Écarts au plan et pièges découverts
- Le statut annonçait `sbin` 32/35 (`atof`, `toupper`) : `atof` était déjà résolu (module `kernel/dev`).
- **`prototype-static` produisait un faux positif** : `case CS5: output("cs5");` était pris pour
  un prototype (`static ` aurait été inséré dans un `switch`). Corrigé avant application ; rejeu
  sur les commits `e4f44f8` et `0fb4fa0` identique (8 fichiers). Pour `bin` : relire les
  occurrences de `prototype-static` de la simulation avant `--apply`.
- `ctype.h` Lepton : remappage `#ifdef` (pas `#ifndef`) : sans `<ctype.h>` système, `tolower`,
  `toupper`… ne sont pas déclarés. Même cause probable pour `tolower` dans `bin` (cf. statut) :
  appliquer le même schéma.
- Aucun preset ne lie `stty`, `xmodem`, `slipd`… : compilés par mass_compile seulement.

## Non transmis volontairement
- Dette consignée au statut sans correction : `stty.c:476/496` (pointeur comparé à `'\0'`),
  `ifconfig` (`if_flags` non initialisé, dette 3b), `ARG_LEN_MAX`.
- `#pragma message` de `compress.c` : valide sous GCC, inactif sur ARM (`INT_MAX` 32 bits).
- Détail : `git log migration/etape-4-sbin`.
