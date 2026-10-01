# Handoff étape 4, module `lib` → modules suivants

État au 2026-10-01 : module fait sur `migration/etape-4-lib`, en attente de validation
utilisateur. Module suivant : `sbin`. Procédure générale : `handoff/etape-4-outillage.md`.

## Résultat du module
- Périmètre : 46 fichiers actifs (`libc`, `librt`, `pthread`) ; `lib-nxpnfc` et les autres
  fichiers différés ne sont pas traités.
- `transform_iar.py` : `garde-cible-gelee` (3 fichiers : `ctype.c`, `ctype.h`, `stdio.h`) et
  `garde-compilateur` (4 fichiers : `printf.c`, `stdio.c`, `unistd/io.c`, `librt/mq.c`) ;
  34 occurrences automatiques, 0 résiduelle ; copies d'origine sous `scion/legacy/sys/root/src/lib/`.
- `audit_isa_ifdef.py` : `lib` 5 → **0** ; total 12 → **7** (4 fichiers : `sbin` 4, `tauon-basic` 3).
- `audit_iar.py` : `lib` 0 (inchangé) ; actif 92 (Lepton 36, tiers 56).
- `mass_compile.sh` : `lib` 27/27, périmètre 340/348 (inchangé).
- `gcc -E -P` identique à chaque commit : presets hôte, hard, soft, et mass_compile (`lib`).
- `ci/run.sh` vert (hôte 5/5, smoke, net, KAL hard 15/15 et soft 11/11).

## Décisions actées pendant le module (2026-10-01)
- `BUFSIZ` : taille par défaut déplacée de `stdio.h` vers `kal/arch/<isa>/kal_arch_conf.h`
  (`__KERNEL_STDIO_PRINTF_BUFSIZ` sous `#ifndef` : le mkconf de la carte reste prioritaire, car
  `kernel_mkconf.h` est inclus avant `kal_arch_conf.h`) ; hôte 256, armv7m et armv6m 128, entre
  parenthèses comme avant ; branche morte `__AS386_16__` (ELKS/bcc) retirée.

## Artefacts produits (manifeste)
| Fichier | Contenu, quand le lire |
|---|---|
| `tools/migration/stdio_bufsiz.py` | script du commit sémantique `BUFSIZ` (assertions, non rejouable sur l'état final) |
| `scion/legacy/sys/root/src/lib/…` | copies d'origine (6 fichiers), gelées par la règle générale de `code-gele.md` (étape 4), supprimées à l'étape 6 |
| `kal/arch/{host,armv7m,armv6m}/kal_arch_conf.h` | + `__KERNEL_STDIO_PRINTF_BUFSIZ` |

## Écarts au plan et pièges découverts
- `perimetre.csv` donne des chemins `src/…` relatifs à `sys/root/` : préfixer `sys/root/`
  pour `transform_iar.py --files-from`.
- Le preset hôte ne compile aucun fichier de `lib` ; seules les bases QEMU et mass_compile
  couvrent le module (l'hôte voit `kal_arch_conf.h`).
- Pour comparer mass_compile à l'état d'avant, l'instantané a été pris par `git stash -u`, `scion
  graft`, `mass_compile.sh --only … --db-out`, `--cpp-snapshot`, puis `stash pop` et `graft` :
  prendre l'instantané **avant** la première règle évite ce détour.
- `stdio.h` est ASCII : un commentaire accentué l'aurait rendu Latin-1 ; commentaire laissé sans accent.
- `visibility("hidden")` de `stdin`/`stdout`/`stderr` et `open` (`stdio.c`, `io.c`) conservés tels
  quels (branches GCC levées sans correction).

## Non transmis volontairement
- Détail : diffs des commits (`git log migration/etape-4-lib`), copies `legacy/`.
