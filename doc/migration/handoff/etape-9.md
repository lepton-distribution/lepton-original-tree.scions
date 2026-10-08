# Handoff étape 9 → 10 (hôte macOS → cartes depuis macOS)

État au 2026-10-08 : tâches 1 à 7 faites sur le Mac Intel (macOS 15.8.1), branche locale
`migration/etape-9` non fusionnée. **Reste avant clôture** : `ci/run.sh` vert sur Debian (critère
de l'étape, ORCHESTRATION §5) et validation de l'utilisateur.

## Réponses aux prérequis de l'étape 10
- Rootstock du Mac : `~/lepton` (`/Users/cle_d_anton/lepton`), comme sur Debian.
- OpenOCD : `/opt/local/bin/openocd`, 0.12.0 (MacPorts `+ftdi`) ; un OpenOCD 0.12.0 Homebrew
  existe aussi (`/usr/local/bin`), masqué par `/opt/local/bin` : vérifier `command -v openocd`.
  Commande inchangée : `cmake --build --preset <carte>-embos --target flash` (`debug/openocd-*.cfg`).
- gdb : `arm-none-eabi-gdb` 15.2.90 de la toolchain Arm 14.2.Rel1
  (`~/opt/arm-gnu-toolchain-14.2.rel1-darwin-x86_64-arm-none-eabi/bin`, dans le `PATH` par
  `~/.zprofile`) ; `debug/gdbinit-*` non rejoués sur le Mac.
- Comparaison des `.bin` : **les 14 diffèrent** de `handoff/etape-8.md` (newlib « 4.4.0 » au lieu
  de 4.5.0.20241231, écart accepté) ; détail et tailles : `etape-9-bin-macos.md`. Les firmwares
  carte du Mac ne sont donc validés par aucun banc : c'est l'objet de l'étape 10.

## Tâches 4 à 7 (session 2)
- Tâche 4 : `host` avec Apple clang 17 ; une correction : groupe `RESCAN` vide sous `APPLE`
  (`cmake/isa/host.cmake`, `31c13e2`) ; mklepton Mach-O 64-bit x86_64 ; `ctest -L host` 6/6,
  `host.mklepton_empreintes` identique à la référence ; expat = SDK (`/usr/lib/libexpat.1.dylib`) ;
  `-nostdinc` + en-têtes de clang ; `host_posix.c` et `m` sans retouche.
- Tâche 5 : décision `net` appliquée (`3b25e72`) ; six presets QEMU sous QEMU 11.1.2 : fumée
  verte, banc KAL 18/18 (M4F, M7) et 14/14 (soft), embOS et FreeRTOS ; listes de tests = Debian
  moins `net.ping_ftpd` (seule condition d'hôte dans le CMake : `grep CMAKE_HOST`).
- Tâche 6 : `SOURCE_DATE_EPOCH=0 ci/run.sh` après `clean` : vert de bout en bout (8 presets carte
  construits, `memoire.py`, trunk sans fichier régulier) ; mesure `5e90c8d`.
- Tâche 7 : `doc/BUILDING.md` §3 ter « Hôte macOS » ; propositions `CLAUDE.md` et skill : à
  trancher par l'utilisateur (compte rendu de session).

## Décisions actées pendant l'étape 9 (détail : MIGRATION-STATUS)
- Rootstock `~/lepton` ; MacPorts ; écart newlib accepté ; label `net` non exécuté sur macOS
  (mis en œuvre avec `CMAKE_HOST_APPLE`, `APPLE` étant faux en build croisé) ; Python/scion du Mac
  conservés.

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `scripts/install-macos.sh` | prérequis du Mac ; rejouable |
| `doc/BUILDING.md` §3 ter | procédure macOS, écarts avec Debian, pièges |
| `doc/migration/etape-9-bin-macos.md` | SHA-256 et tailles des 14 `.bin` du Mac, répartition par origine |
| `tools/migration/map_origine.py` | octets par origine depuis les `.map` ; à rejouer sur Debian pour localiser l'écart |
| `$LEPTON_BUILD/ci/` (Mac) | artefacts et `memoire.csv` de `ci/run.sh` à date fixe |
| `$LEPTON_BUILD/etape9-*.log`, `etape9-bin-sha256.txt` | journaux des presets QEMU et de la CI, empreintes |

## Écarts au plan et pièges découverts
- `$<LINK_GROUP:RESCAN>` : erreur de configuration sous Apple (« not supported for the 'C' link
  language ») — seule hypothèse du tableau de la tâche 4 qui s'est vérifiée.
- `cc` et `clang` sont le même Apple clang : la seconde passe `host` de `ci/run.sh` ne teste pas
  un second compilateur sur le Mac (gcc et clang restent couverts sur Debian).
- Avertissements Apple clang sur le noyau (`-Wvisibility` `struct stat` dans `vfstypes.h`,
  `-Wvarargs` de `kernel_io.c`…) : non comparés à clang 19 de Debian, aucun traité.
- `.DS_Store` dans le clone (ignorés par git, non greffés) : Finder ouvert sur le clone ; sans effet.
- Mac (environnement, hors dépôt) : voir la session 1 — MacPorts jamais initialisé, CLT avec un
  `c++/v1` orphelin, git 2.18 dans `/usr/local/bin`, `sudo` par l'utilisateur (`! …`).
- Le shell de l'agent est zsh : `$var` non découpé en mots ; passer par `bash <<'EOF'`.

## Non transmis volontairement
- Diagnostic de l'environnement du Mac (session 1) : historique de la conversation et journaux
  `/opt/local/var/macports/logs/`.
- Taille par section des `.bin` Debian : non disponible sur le Mac ; à produire sur Debian par
  `map_origine.py` si l'écart doit être localisé.
