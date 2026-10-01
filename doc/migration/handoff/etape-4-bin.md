# Handoff étape 4, module `bin` → modules suivants

État au 2026-10-01 : module fait sur `migration/etape-4-bin`, validé par l'utilisateur le
2026-10-01 et fusionné. Module suivant : `sys/user/tauon-basic`. Procédure générale :
`handoff/etape-4-outillage.md`.

## Résultat du module
- Périmètre : 13 fichiers actifs (8 `.c`) de `sys/root/src/bin` ; mongoose classé tiers.
- `transform_iar.py --rule garde-cible-gelee` (élargie à `WIN32`) : `test2.c`, branche
  `Sleep(10)` retirée, copie d'origine sous `scion/legacy/sys/root/src/bin/`.
- Corrections sémantiques (une par commit) : `httpc.c` (`<ctype.h>`, `perror` → `fprintf` +
  `strerror` Lepton) ; mongoose (`<ctype.h>`, `lib/libc/string/string.h` dans le bloc
  `__tauon_posix__`, exception D1a) ; `telnetd.c`, `test2.c` (`accept` : `uint32_t*`).
- `gcc -E -P` identique (hard, soft, mass_compile `bin`) après le commit mécanique ; les
  sources des presets QEMU ne sont pas touchés (159/159 identiques en fin de module).
- `mass_compile.sh` : `bin` **8/8** ; périmètre **346/348** (restent les 2 de `tauon-basic`).
- `audit_iar.py` : actif 92 inchangé (Lepton 36, tous hors `bin`) ; `audit_isa_ifdef.py` : 3.
- `ci/run.sh` vert (outils 49 tests, hôte 5/5, smoke, net, KAL hard 15/15 et soft 11/11).

## Décisions actées pendant le module (2026-10-01, plan approuvé)
- Mongoose : exception étroite à D1a (bloc d'inclusions Lepton seulement).
- `perror` : remplacement local dans `httpc.c` ; `lib/libc` non modifié.
- `garde-cible-gelee` couvre la macro `WIN32` (D3a) ; `_WIN32` non visé (mongoose, tiers).

## Gardes WIN32 restantes : reportées à l'étape 6 (décision 2026-10-01)
- La règle élargie trouve des gardes `WIN32` actives dans des modules **déjà validés** :
  `kernel/core` (`core-segger/fork.c`, `interrupt.h`, `kernel_compiler.h`,
  `kernel_pthread.h`, `kernelconf.h`) et `kernel/dev` (`dev_ftl.c`). Non transformées à l'étape 4 ;
  à retirer à l'étape 6 avec le code gelé. Détail : simulation dans
  `residuel-etape4.md`. `tauon-basic` (`timers.c`, `dlmalloc.c`) : à traiter à son module.

## Artefacts produits (manifeste)
| Fichier | Contenu, quand le lire |
|---|---|
| `tools/migration/transform_iar.py` | `WIN32` dans `GELE` ; `directive()`/`lignes_masquees()` : directives en commentaire ignorées |
| `tools/migration/tests/test_transform_iar.py` | `test_win32_compilateur`, `test_directive_dans_commentaire` |
| `scion/legacy/sys/root/src/bin/test2.c` | copie d'origine, gelée, supprimée à l'étape 6 |

## Écarts au plan et pièges découverts
- **Défaut de l'outil, corrigé avant le commit** : `test2.c` contient `#endif*/` (une garde IAR
  dans un bloc commenté). `garde-cible-gelee` retirait la ligne, donc la fin du commentaire
  (le reste du fichier aurait été commenté). Les règles de gardes reconnaissent désormais les
  directives sur le texte masqué. Simulation du périmètre actif identique à part ces 4 gardes
  commentées de `test2.c`. Pour `tauon-basic` : relire le diff de chaque `--apply`.
- `--apply` lancé depuis le trunk : le hook `lepton_guard.py` le refuse. Lancer depuis le clone
  (l'outil résout lui-même les chemins vers `scion/`).
- `copie_legacy` n'écrase jamais une copie existante : en cas de nouvelle transformation d'un
  fichier déjà copié, `legacy/` garde l'original le plus ancien.
- `mass-compile.csv` ne donne que la première erreur : 5 fichiers en échec cachaient
  14 déclarations implicites (`ctype`, `strerror`).

## Non transmis volontairement
- Dette consignée au statut : `addrlen` de `telnetd.c` non initialisé ; `error()` inutilisée
  dans `httpc.c` ; aucun preset ne lie `httpc`, mongoose, `telnetd`, `test2`.
- Détail : `git log migration/etape-4-bin`.
