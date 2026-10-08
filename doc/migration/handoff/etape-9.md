# Handoff étape 9 (hôte macOS) — intermédiaire, session 1 → session 2

État au 2026-10-08 : branche `migration/etape-9` (locale), tâches 1 à 3 faites, tâches 4 à 7 à
faire. Mac Intel, macOS 15.8.1. Ce fichier deviendra le handoff étape 9 → 10 en fin d'étape.

## Pour reprendre (session 2 : tâches 4 à 7)
- **PATH** : la toolchain Arm n'est pas dans le PATH des nouveaux shells tant que l'utilisateur
  n'a pas ajouté à `~/.zprofile` :
  `export PATH="$HOME/opt/arm-gnu-toolchain-14.2.rel1-darwin-x86_64-arm-none-eabi/bin:$PATH"`
  (le script ne modifie pas le shell). Vérifier `command -v arm-none-eabi-gcc` d'abord.
- Tâche 4 : `cmake --preset host` avec Apple clang ; points à vérifier dans ETAPE-9 §4
  (`LINK_GROUP:RESCAN`, `-print-file-name=include`/`-nostdinc`, expat du SDK, POSIX de
  `host_posix.c`, `m`). Attention : CMake cherche aussi dans `/opt/local` (Darwin) — les ports
  i386 de 2008 sont désinstallés, mais vérifier quel expat est retenu.
- Tâche 5 : appliquer la décision `net` (2026-10-08) : `cmake/components/firmware.cmake:147`
  (`if(... AND NOT APPLE)` + `message(STATUS …)`) et `ci/run.sh:103-104` (`ctest -L net
  --no-tests=error` échouerait sinon). QEMU **11.1.2** sur le Mac (10.0.13 sur Debian) :
  premier écart à surveiller sur la fumée et le banc KAL.
- Tâche 6 : `--target clean` puis `SOURCE_DATE_EPOCH=0 ci/run.sh`, comparer les SHA-256 des
  `.bin` au tableau de `handoff/etape-8.md` ; différences attendues (newlib « 4.4.0 »).
- `export PYTHONDONTWRITEBYTECODE=1` avant tout `ctest` hors `ci/run.sh`.

## Réponses aux prérequis (ETAPE-9) et tâches faites
- Étape 8 close (handoff `etape-8.md`) ; MacPorts installé et réparé ; macOS 15.8.1 consigné.
- Rootstock `~/lepton` (décision 2026-10-08) ; embOS 5.20.0.0 présent dans
  `third_party/embos/cortexm-gcc/5.20.0.0/` (copié par l'utilisateur).
- Tâche 1 : `install-macos.sh --with-debug-tools` rejoué jusqu'au bout ; versions dans
  « Versions épinglées » ; corrections : commit `3cc34c5`.
- Tâche 2 : `scion rootstock-information` = 2 scions ; **4019 feuilles** (= étape 8) ; clone
  propre ; `find -L trunk -type l` vide ; aucune collision de casse (`git ls-files`).
- Tâche 3 : `lepton-env.sh` (`rev-parse --abbrev-ref`, commit `f487de4`) vert sous zsh et bash
  3.2 (cinq variables) ; hook : Write et redirection vers le trunk refusés ; 5 `.DS_Store`
  supprimés du trunk (et celui du rootstock) ; contrôle « trunk sans fichier régulier » vert.

## Décisions actées pendant la session 1 (détail : MIGRATION-STATUS)
- MacPorts ; 14 ports i386 de 2008 désinstallés ; Python/scion conservés ; écart newlib
  accepté ; label `net` non exécuté sur macOS (à appliquer, tâche 5).

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `scripts/install-macos.sh` | prérequis du Mac ; rejouable (ce qui est en place est sauté) |
| `$LEPTON_BUILD/install-macos.log`, `install-macos-2.log` | 1er passage (échec graphviz), 2e passage complet |
| `~/opt/arm-gnu-toolchain-14.2.rel1-darwin-x86_64-arm-none-eabi/*-manifest.txt` | options de configuration de la toolchain et de newlib (Arm) |

## Écarts au plan et pièges découverts (environnement du Mac, hors dépôt)
- **MacPorts jamais initialisé** : registre root, migré un palier par lancement en root
  (1.211 → 1.215) : `sudo port -v selfupdate` puis `sudo port -q installed` répété ; source
  rsync non signée à remplacer dans `sources.conf`.
- **14 ports i386 de 2008** (expat 2.0.1, zlib 1.2.3, pkgconfig…) dans `/opt/local` : `pkg-config`
  i386 en tête du PATH, `expat.h` de 2008 visible de CMake → désinstallés.
- **C++ cassé par un reste des CLT** : `CommandLineTools/usr/include/c++/v1` orphelin (59
  fichiers 2020-2023, aucun paquet propriétaire) masquait la libc++ du SDK ; tout `clang++`
  échouait (`'cstdint' file not found`, échec de libheif → graphviz). Supprimé par
  l'utilisateur. Symptôme à reconnaître sur un autre Mac.
- `graphviz` sans paquet binaire darwin 24 : rendu facultatif dans le script.
- git 2.18 (`/usr/local/bin/git`, 2018) devant le git d'Apple 2.50.1 : `--show-current`
  absent → script corrigé ; retrait du lien laissé à l'utilisateur.
- Le shell de l'agent est zsh : `$var` non découpé en mots ; passer par `bash <<'EOF'`.
- `sudo` exige un mot de passe : les commandes root passent par l'utilisateur (`! …`).

## Non transmis volontairement
- Diagnostic détaillé du registre MacPorts et des CLT : historique de la conversation ; les
  gestes à refaire sont ci-dessus. Journaux MacPorts : `/opt/local/var/macports/logs/`.
