# Étape 9 — Hôte macOS (Intel) : noyau statique, mklepton, QEMU

## Contexte

Lepton se construit et se teste sur un seul hôte, Debian 13 x86_64. Cette étape ajoute un second
hôte, macOS sur Mac Intel (x86_64), sans carte : seuls changent le système et le compilateur de
l'hôte (Apple clang, Mach-O, éditeur de liens d'Apple, outils BSD, APFS). Les cartes sont
l'objet de l'étape 10.

Archétype : infrastructure. Le second hôte s'insère dans les mécanismes en place (presets,
CTest, `ci/run.sh`, `lepton-env.sh`) ; il n'en crée pas de parallèles. Debian reste l'hôte de
référence.

## Prérequis

- Étape 8 close : preset `host` en 64 bits, `tests/host/mklepton_empreintes.sha256` versionné,
  build clang vert sur Debian ; `handoff/etape-8.md`.
- Mac Intel, **MacPorts** installé (Homebrew ne fournit plus de paquets binaires pour Intel
  depuis septembre 2026) ; version de macOS à consigner.
- Rootstock sur le Mac : `<À CONFIRMER>` (proposition : `~/lepton`, comme sur Debian).
- Paquet embOS-Classic V5.20.0.0 Cortex-M GCC copié par l'utilisateur dans
  `<ROOTSTOCK>/third_party/embos/cortexm-gcc/5.20.0.0/` (licence SFL : hors git, jamais téléchargé
  ni redistribué par l'agent).
- Claude Code lancé sur le Mac par `scripts/claude-lepton.sh`. Branche `migration/etape-9`.

## Tâches

### 1. Prérequis de l'hôte : `scripts/install-macos.sh`

Pendant de `install-debian.sh`, même structure, même récapitulatif final des versions. Le script
est **fourni** (écrit le 2026-10-07 sans Mac : syntaxe et `shellcheck` seulement, jamais exécuté
sous macOS). Le rejouer, corriger ce qui échoue, sans en changer les choix ci-dessous.

| Élément | Debian | macOS |
|---|---|---|
| CMake, Ninja, Python 3, pipx | apt | MacPorts (`cmake`, `ninja`, `python313`, `pipx`) |
| expat | `libexpat1-dev` | SDK de macOS ; MacPorts seulement si l'en-tête manque |
| `scion` | pipx, tag `0.5.0.1` | idem, même tag |
| QEMU | `qemu-system-arm` 10.0.13 | MacPorts `qemu +target_arm` (variante non activée par défaut : compilation sur place possible) ; `mps2-an386` et `mps2-an500` exigées |
| Toolchain ARM | `gcc-arm-none-eabi` 14.2.1, newlib 4.5.0.20241231 | Arm GNU Toolchain 14.2.Rel1, hôte `darwin-x86_64` (site d'Arm), pas la formule Homebrew (non épinglable) |
| OpenOCD, gdb | `openocd` 0.12.0, `gdb-multiarch` | MacPorts `openocd` ; `arm-none-eabi-gdb` de la toolchain |

- Le script télécharge l'archive d'Arm, en vérifie le SHA-256, la dépose sous `~/opt` (ou
  `--toolchain-dir`), contrôle `libc_nano.a` pour cortex-m4 hard-float et relève la version de
  newlib (`_NEWLIB_VERSION`). Il affiche la ligne `PATH` à ajouter, sans modifier le shell.
- **Point d'arrêt, attendu** : la note de version d'Arm indique une newlib construite depuis le
  tronc (révision `7923059`), et non la 4.5.0.20241231 de Debian. Présenter les deux versions
  relevées et faire trancher avant de construire une cible : accepter l'écart (les firmwares du
  Mac diffèrent alors de ceux de Debian, l'étape 10 les valide) ou chercher une autre toolchain.
- Apple Silicon : le script choisit l'archive `darwin-arm64` et y prend Homebrew par défaut,
  mais ce plan ne valide que le Mac Intel.
- `sudo` pour `port install` seulement ; rien d'i386 ; pas de règles udev. Paquets sous
  `/opt/local` : vérifier que CMake y trouve ce qu'il cherche sans chemin codé en dur.

### 2. Arbre des sources

- Suivre `doc/BUILDING.md` §2 sur le Mac. Contrôles : `scion rootstock-information` ; nombre de
  feuilles du trunk égal à celui du handoff de l'étape 8 ; `git status` propre dans le clone
  juste après le clonage ; aucun lien cassé (`find -L "$LEPTON_TRUNK" -type l` vide).
- Volume APFS insensible à la casse : aucune collision dans le dépôt au 2026-10-07
  (`git ls-files`, comparaison sans casse, `ae85f06`). Le revérifier ; une collision est un
  point d'arrêt (volume sensible à la casse ou renommage).

### 3. Scripts et hook

- `source scripts/lepton-env.sh` depuis zsh et depuis bash 3.2 : les cinq variables exportées.
- `scripts/claude-lepton.sh` ; hook `.claude/hooks/lepton_guard.py` : rejouer le contrôle de
  l'étape 0 (une écriture dans le trunk est refusée).
- Tout script du dépôt doit fonctionner avec les outils BSD de macOS, sans exiger coreutils
  GNU. Un script corrigé doit rester vert sur Debian.

### 4. Preset `host`

- `cmake --preset host && cmake --build --preset host`, Apple clang.
- Points à vérifier, non testés à ce jour sur macOS — ce sont des hypothèses, pas des constats :

  | Point | Où corriger si nécessaire |
  |---|---|
  | `$<LINK_GROUP:RESCAN,…>` (`cmake/components/kernel.cmake`) avec l'éditeur de liens d'Apple | `cmake/isa/host.cmake`, sous condition `APPLE` |
  | `lepton_freestanding` : `-print-file-name=include` et `-nostdinc` avec Apple clang | `cmake/lepton_target.cmake`, `cmake/isa/host.cmake` |
  | expat : en-tête et bibliothèque trouvés | `cmake/components/mklepton.cmake` |
  | `_GNU_SOURCE`, appels POSIX de `kernel/dev/arch/host/common/host_posix.c` | ce fichier |
  | bibliothèque `m` (`LEPTON_SYSTEM_LIBS`) | `cmake/isa/host.cmake` |

- Toute correction reste dans `cmake/isa/host.cmake`, `cmake/components/mklepton.cmake`,
  `kernel/core/arch/host/`, `kernel/dev/arch/host/`, `kal/arch/host/`, `tools/mklepton/`.
  **Point d'arrêt** avant de modifier un fichier du noyau commun.
- `ctest --preset host -L host` : les six tests, dont `host.mklepton_empreintes` (référence
  produite sur Debian en `-m32`) et `host.ufs_format_arm`.

### 5. Presets QEMU

- Construire les six presets QEMU (`qemu-mps2-an386-{embos,freertos}[-soft]`,
  `qemu-mps2-an500-{embos,freertos}`) ; `ctest -L smoke` et `ctest -L kal` sur chacun.
- Label `net` : `tests/net_qemu.py` crée un tap dans un espace de noms (`unshare`), propre à
  Linux. Proposition, **à faire acter** : sur macOS, le test n'est pas créé, CMake l'annonce à
  la configuration et `ci/run.sh` affiche « net : non exécuté sur macOS » ; le réseau est
  couvert sur carte à l'étape 10 et sous QEMU par Debian. Ne pas le désactiver sans cet accord.

### 6. `ci/run.sh` sur macOS

- Vert de bout en bout, les huit presets carte construits, `ci/memoire.py` compris.
- Mesure, sans critère : SHA-256 des `.bin` des 14 presets, comparés à ceux du handoff de
  l'étape 8. Des binaires différents sont attendus si la toolchain n'est pas construite comme
  celle de Debian ; noter quelles sections diffèrent (`arm-none-eabi-size`, `.map`).

### 7. Documentation

- `doc/BUILDING.md` : section « Hôte macOS » (prérequis, écarts avec Debian, ce qui n'y tourne pas).
- Proposer à l'utilisateur, sans les appliquer d'office : la ligne « Environnement de build »
  de `CLAUDE.md` §2 et la convention « Environnement » du skill `lepton-portage-instructions`.

## Critères de validation

- [ ] `scripts/install-macos.sh` rejoué sur le Mac sans intervention ; versions consignées.
- [ ] `file "$LEPTON_BUILD/host/mklepton"` : « Mach-O 64-bit executable x86_64 ».
- [ ] `ctest --preset host -L host` : 6/6 ; empreintes identiques à la référence Debian.
- [ ] Six presets QEMU : `ctest -L smoke` et `-L kal` verts, mêmes listes de tests que sur
      Debian (`ctest -N`).
- [ ] `ci/run.sh` vert sur macOS ; trunk sans fichier régulier.
- [ ] `ci/run.sh` toujours vert sur Debian après fusion (aucune régression du premier hôte).
- [ ] `git diff master --stat -- scion/sys/root/src` : fichiers sous `arch/host/` seulement, ou
      écart accepté au point d'arrêt.
- [ ] `doc/BUILDING.md` : un développeur construit le preset `host` et lance la fumée QEMU sur
      un Mac Intel en suivant la section, sans aide.

## Pièges connus

- **Mac Intel en fin de support** : Apple abandonne Intel à partir de macOS 27 et Homebrew ne le
  prend plus en charge ; cet hôte ne recevra plus de mises à jour d'outils. Consigner les
  versions installées, ne pas compter sur leur renouvellement.
- **Quarantaine** : une toolchain téléchargée par navigateur porte l'attribut
  `com.apple.quarantine` ; symptôme « impossible d'ouvrir … développeur non vérifié » au premier
  `arm-none-eabi-gcc`. Parade : retirer l'attribut sur le répertoire de la toolchain.
- **`.DS_Store`** : ouvrir le trunk dans le Finder y crée un fichier régulier ; le contrôle final
  de `ci/run.sh` échoue. Supprimer le fichier ; ne pas affaiblir le contrôle.
- **`__pycache__`** dans le trunk : `export PYTHONDONTWRITEBYTECODE=1` avant tout `ctest` hors
  `ci/run.sh` (CLAUDE.md §3).
- **Outils BSD** : `sed -i` exige un suffixe, `readlink -f`, `stat -c`, `date -d`, `sha256sum`
  et `nproc` n'existent pas ; bash est en 3.2 (ni `mapfile`, ni tableaux associatifs).
- **Mach-O** : `__attribute__((section("nom")))` exige `"segment,section"` ; pas d'alias de
  symbole (`__attribute__((alias))`) ; `--start-group`, `--gc-sections` inconnus de l'éditeur de
  liens. Aucun de ces usages dans le noyau statique au 2026-10-07.
- **Deux Python** (outils Xcode, MacPorts) : `pipx` et `cmake` doivent voir le même `python3`.
- **`LEPTON_MKLEPTON`** des presets croisés pointe `../build/host/mklepton` : construire le preset
  `host` avant tout preset croisé, comme sur Debian.

## À la fin de l'étape

`MIGRATION-STATUS.md` : ligne de l'étape 9 ; « Versions épinglées » : colonne ou lignes macOS
(macOS, Apple clang, CMake, Ninja, QEMU, toolchain ARM, newlib, OpenOCD) ; décisions des points
d'arrêt. `handoff/etape-9.md`, qui donne pour l'étape 10 : commande et version d'OpenOCD et de
`arm-none-eabi-gdb`, emplacement du rootstock, résultat de la comparaison des `.bin`.
