# Construire Lepton (GCC, Linux et macOS) et flasher les cartes

Procédure de bout en bout sur un hôte Debian 13 x86_64 (second hôte : macOS sur Mac Intel, §3 ter) : arbre des sources, build, tests sous
QEMU, CI, flash et test sur les cartes. Chaîne unique : `arm-none-eabi-gcc` et CMake (IAR retiré à
l'étape 6 ; état précédent : tag local `legacy-iar`). Versions épinglées :
`doc/migration/MIGRATION-STATUS.md`.

## Cibles : cœurs × cartes × statut

| Cœur (ISA) | Machine QEMU / carte | Preset | Statut |
|---|---|---|---|
| Cortex-M4F (`armv7m`) | QEMU `mps2-an386` | `qemu-mps2-an386-embos` (hard-float), `qemu-mps2-an386-embos-soft` | QEMU : fumée, réseau, banc KAL verts (CI) |
| Cortex-M4F (`armv7m`) | NUCLEO-F439ZI (validée sur NUCLEO-F429ZI) | `nucleo-f439zi-embos` | validé sur carte (réseau, endurance) ; build en CI |
| Cortex-M7 (`armv7m`) | QEMU `mps2-an500` | `qemu-mps2-an500-embos` | QEMU : fumée, réseau, banc KAL verts (CI) |
| Cortex-M7 (`armv7m`) | STM32F746G-DISCO | `stm32f746g-disco-embos` | validé sur carte (réseau, endurance 4 h) ; build en CI |
| Cortex-M4 sans FPU (`armv7m`, soft) | NUCLEO-WL55JC1 (CPU1 seul) | `nucleo-wl55jc1-embos` | validé sur carte (radio FSK 868 MHz, endurance 1 h, sans réseau IP) ; build en CI |
| Cortex-M0+ (`armv6m`) | SAMD21 Xplained Pro | `samd21-xplained-pro-embos` | validé sur carte (endurance 1 h, sans réseau) ; build en CI |
| hôte x86_64 (gcc, clang) | noyau statique, mklepton | `host` | tests hôte verts avec gcc et clang (CI) |

Micro-noyau : embOS (port GCC Segger) sur toutes les cibles ARM. Cortex-M3 (`mps2-an385`) : fichier
cœur présent, hors périmètre de validation. Ajouter un cœur ou une carte :
`doc/migration/ajout-coeur.md`.

## 1. Prérequis de l'hôte

```bash
git clone https://github.com/lepton-distribution/lepton-original-tree.scions.git /tmp/lepton-tree
/tmp/lepton-tree/scripts/install-debian.sh --with-debug-tools   # toolchain, CMake, QEMU, OpenOCD, udev
```

`--with-debug-tools` installe OpenOCD, `gdb-multiarch` et les règles udev de la sonde ST-LINK
(l'utilisateur doit appartenir au groupe `plugdev` ; rebrancher la carte après installation).

embOS (Segger, licence SFL : évaluation, non redistribuable) n'est pas dans git : déposer le paquet
« embOS-Classic V5.20.0.0 Cortex-M GCC » dans `<ROOTSTOCK>/third_party/embos/cortexm-gcc/5.20.0.0/`
(répertoires `Start/Inc`, `Start/Lib`).

## 2. Arbre des sources (scion, étape 0)

```bash
pipx install "git+https://github.com/lepton-distribution/seed.scions.git@0.5.0.1"
mkdir -p ~/lepton && cd ~/lepton                 # rootstock
scion rootstock-install
scion seed-add --version original-tree https://github.com/lepton-distribution/lepton-seed.scions.git
scion graft
```

Résultat : `depots/lepton/original/master/` (clone git, **seul endroit où l'on édite**) et
`trunk/` (vue de liens, lecture et build ; ne jamais y écrire). Après tout ajout de fichier dans
le clone : `scion graft`.

```bash
cd ~/lepton/depots/lepton/original/master
source scripts/lepton-env.sh      # LEPTON_ROOTSTOCK, LEPTON_TRUNK, LEPTON_CLONE, LEPTON_BUILD, LEPTON_EMBOS_ROOT
```

## 3. Compiler et tester sous QEMU

Les répertoires de build sont sous `$LEPTON_BUILD/<preset>` (hors du trunk). Le preset `host`
construit d'abord `mklepton` (génération de la configuration et du rootfs) :

```bash
cd "$LEPTON_TRUNK"
cmake --preset host && cmake --build --preset host
cmake --preset qemu-mps2-an386-embos && cmake --build --preset qemu-mps2-an386-embos
ctest --preset qemu-mps2-an386-embos -L smoke     # démarrage → lsh → uname -a
```

Cortex-M7 sous QEMU : preset `qemu-mps2-an500-embos` (mêmes commandes).

## 3 bis. Intégration continue

`ci/run.sh` (depuis le clone) est la non-régression complète, à rendre verte avant chaque commit :

- tests unitaires des outils de migration ; garde `audit_iar.py --garde` (aucun IAR-isme dans le
  code Lepton du périmètre actif) ;
- preset `host` : `ctest -L host` ;
- presets QEMU (M4F hard et soft-float, M7) : `ctest -L smoke`, `-L net` (tap dans un espace de
  noms utilisateur, sans droits root), `-L kal` ;
- presets carte : build seul (les tests `board` exigent la sonde, voir ci-dessous) ;
- occupation mémoire de chaque exécutable (`--print-memory-usage`) archivée dans
  `$LEPTON_BUILD/ci/memoire.csv`, échec si une région dépasse `LEPTON_MEM_ALERT` % (90 par
  défaut) ; artefacts `.elf`, `.bin`, `.map` dans `$LEPTON_BUILD/ci/artefacts/<preset>/`.

Conteneur de référence : `ci/Dockerfile` (Debian 13, `scripts/install-debian.sh`), rootstock monté
entier ; commandes en tête du fichier. **Non encore construit ni exécuté** (aucun moteur de
conteneur sur l'hôte de validation) : la CI est validée en natif.

Compilation de masse du périmètre actif (hors CI, après une transformation) :
`tools/migration/mass_compile.sh`.

## 3 ter. Hôte macOS (Mac Intel, étape 9)

Second hôte, validé sur un Mac Intel (macOS 15.8.1) ; Debian reste l'hôte de référence. Apple
Silicon : non validé. Versions relevées : « Versions épinglées » de `MIGRATION-STATUS.md`.

**Prérequis.** Outils de ligne de commande Xcode (`xcode-select --install`) et
[MacPorts](https://www.macports.org) installés à la main (Homebrew ne fournit plus de paquets
binaires pour Intel). Puis :

```bash
git clone https://github.com/lepton-distribution/lepton-original-tree.scions.git /tmp/lepton-tree
/tmp/lepton-tree/scripts/install-macos.sh --with-debug-tools   # sudo demandé pour port install
```

Le script installe par MacPorts CMake, Ninja, QEMU (`+target_arm`, compilé sur place : long) et
OpenOCD ; il télécharge l'Arm GNU Toolchain 14.2.Rel1 `darwin-x86_64` sous `~/opt` (SHA-256
vérifié) et **affiche la ligne `PATH` à ajouter** à `~/.zprofile` (il ne modifie pas le shell).
expat vient du SDK de macOS. Python et pipx : ceux déjà présents sont conservés. embOS : comme
au §1. Arbre des sources et `lepton-env.sh` : §2, à l'identique (zsh ou bash 3.2).

**Build et fumée QEMU** : commandes du §3, à l'identique (preset `host` avec Apple clang, avant
tout preset croisé). `ci/run.sh` (§3 bis) tourne aussi sur le Mac.

**Écarts avec Debian.**

| Sujet | macOS |
|---|---|
| Compilateur hôte | Apple clang (`cc` et `clang` sont le même : les deux passes `host` de `ci/run.sh` sont identiques) ; `mklepton` en Mach-O x86_64 ; sorties identiques à la référence (`host.mklepton_empreintes`) |
| Édition de liens hôte | éditeur de liens d'Apple : pas de `--start-group`, le groupe `RESCAN` du noyau est vide (`cmake/isa/host.cmake`) |
| newlib de la toolchain Arm | « 4.4.0 » (tronc) au lieu de 4.5.0.20241231 : `.bin` différents de ceux de Debian, validés par exécution (écart accepté) |
| QEMU | version MacPorts (11.1.x), plus récente que celle de Debian |
| Label `net` | **non exécuté** : le test crée un tap dans un espace de noms (Linux seul) ; non créé par CMake (message à la configuration), sauté par `ci/run.sh` ; réseau couvert par Debian et sur carte |

**Pièges.** Ne pas ouvrir le trunk dans le Finder (`.DS_Store` : fichier régulier, contrôle final
de `ci/run.sh` en échec ; le supprimer). Toolchain téléchargée par navigateur : retirer
l'attribut `com.apple.quarantine` de son répertoire. `export PYTHONDONTWRITEBYTECODE=1` avant
tout `ctest` lancé à la main. Un git ancien devant celui d'Apple (`/usr/local/bin`) est toléré
par les scripts. Outils de migration non portés (hors CI) : `tools/migration/mklepton_oracle.sh`,
`tools/migration/lib_unresolved.sh` (bash 4, GNU).

## 4. Carte NUCLEO-F439ZI (validée sur NUCLEO-F429ZI)

Brancher la carte par le connecteur USB du ST-LINK (CN1). Le port série virtuel apparaît en
`/dev/ttyACM0` (console de Lepton : USART3, 115200 8N1).

```bash
cd "$LEPTON_TRUNK"
cmake --preset nucleo-f439zi-embos -DLEPTON_BOARD_SERIAL_PORT=/dev/ttyACM0
cmake --build --preset nucleo-f439zi-embos
cmake --build --preset nucleo-f439zi-embos --target flash    # OpenOCD : écriture, vérification, reset
```

Console : `picocom -b 115200 /dev/ttyACM0` (ou tout terminal série), touche Entrée pour l'invite
`lepton#2$`. Tests sur carte (flashent eux-mêmes le firmware puis le banc KAL ; la carte reste
avec `kal_bench` : reflasher par la cible `flash` ensuite) :

```bash
ctest --preset nucleo-f439zi-embos -L board
```

Réseau : la carte prend l'adresse du `.init` de sa configuration
(`sys/user/tauon-basic/etc/nucleo-f439zi/.init`, 192.168.2.5) ; `ftpd` écoute sur le port 21. Le
test réseau sur carte (`board.net` : ping, session FTP, errno) est créé si l'adresse de l'hôte sur
le câble de la carte est fournie, puis lancé avec les autres tests `board` :

```bash
cmake --preset nucleo-f439zi-embos -DLEPTON_BOARD_SERIAL_PORT=/dev/ttyACM0 -DLEPTON_NET_TEST_HOST_IP=192.168.2.20
ctest --preset nucleo-f439zi-embos -R board.net
```

Optimisation : `-Os` pour toutes les cibles Cortex-M (`LEPTON_OPT_LEVEL`, `-g` conservé) ;
`-DLEPTON_OPT_LEVEL=-O0` pour un débogage pas à pas fidèle au source.

Débogage (gdb, OpenOCD, registres de faute) : `doc/migration/debug-gcc.md`.

## 5. Carte STM32F746G-DISCO (étape 6)

Brancher la carte par le connecteur USB du ST-LINK (CN14). Console de Lepton : USART1,
115200 8N1, sur le port série virtuel (`/dev/ttyACM0`). Mêmes commandes que la NUCLEO avec le
preset `stm32f746g-disco-embos` (Cortex-M7 r0p1, FPU simple précision, embOS `_837070`) :

```bash
cd "$LEPTON_TRUNK"
cmake --preset stm32f746g-disco-embos -DLEPTON_BOARD_SERIAL_PORT=/dev/ttyACM0
cmake --build --preset stm32f746g-disco-embos
cmake --build --preset stm32f746g-disco-embos --target flash
ctest --preset stm32f746g-disco-embos -L board      # fumée + banc KAL ; reflasher ensuite
```

Débogage : `debug/openocd-stm32f746g-disco.cfg`, `debug/gdbinit-stm32f746g-disco`. Réseau :
comme la NUCLEO (adresse 192.168.2.5 du `.init`, `ftpd`) ; test sur carte avec
`-DLEPTON_NET_TEST_HOST_IP=<adresse de l'hôte sur le câble>` puis `ctest … -R board.net`.

## 6. Carte SAMD21 Xplained Pro (Cortex-M0+, étape 6)

Brancher la carte par le connecteur USB de l'EDBG (« DEBUG USB », sonde CMSIS-DAP `03eb:2111`).
Console de Lepton : SERCOM3 (PA22/PA23), 115200 8N1, sur le port série virtuel de l'EDBG
(`/dev/ttyACM0`). Configuration minimale sans réseau.

```bash
cd "$LEPTON_TRUNK"
cmake --preset samd21-xplained-pro-embos -DLEPTON_BOARD_SERIAL_PORT=/dev/ttyACM0
cmake --build --preset samd21-xplained-pro-embos
cmake --build --preset samd21-xplained-pro-embos --target flash
ctest --preset samd21-xplained-pro-embos -L board   # fumée + banc KAL ; reflasher ensuite
```

Débogage : `debug/openocd-samd21-xplained-pro.cfg`, `debug/gdbinit-samd21-xplained-pro`
(registres de faute ARMv6-M : `lepton-fault-v6m`).

## 7. Carte NUCLEO-WL55JC1 (Cortex-M4 du CPU1, étape 6)

Brancher la carte par le connecteur USB du STLINK-V3. Console de Lepton : USART2, 115200 8N1.
Désigner la console par son nom stable
`/dev/serial/by-id/usb-STMicroelectronics_STLINK-V3_<n° de sonde>-if02` (l'ordre des `ttyACM`
change) et la sonde par `-DLEPTON_BOARD_STLINK_SERIAL=<n° de sonde>` quand plusieurs cartes sont
branchées. Le CPU2 (Cortex-M0+) n'est jamais démarré ; pas de réseau IP.

```bash
cd "$LEPTON_TRUNK"
cmake --preset nucleo-wl55jc1-embos -DLEPTON_BOARD_SERIAL_PORT=<console> -DLEPTON_BOARD_STLINK_SERIAL=<n°>
cmake --build --preset nucleo-wl55jc1-embos
cmake --build --preset nucleo-wl55jc1-embos --target flash
ctest --preset nucleo-wl55jc1-embos -L board        # fumée + banc KAL ; reflasher ensuite
```

Radio Sub-GHz (FSK 868 MHz, `/dev/radio`, pseudo-binaire `radiotst`) : le test `board.radio`
exige une seconde carte, désignée par `-DLEPTON_RADIO_PEER_STLINK_SERIAL=<n°>` et
`-DLEPTON_RADIO_PEER_SERIAL_PORT=<console>` ; la seconde carte se construit dans un autre
répertoire (`cmake --preset nucleo-wl55jc1-embos -B "$LEPTON_BUILD/nucleo-wl55jc1-embos-b" …`),
sans modifier le preset. Débogage : `debug/openocd-nucleo-wl55jc1.cfg`,
`debug/gdbinit-nucleo-wl55jc1`.

## 8. Documentation Doxygen

```bash
cd ~/lepton/depots/lepton/original/master && source scripts/lepton-env.sh
doxygen doc/Doxyfile          # HTML dans $LEPTON_BUILD/doc/html/index.html
```

La page principale est ce fichier ; le contrat de la KAL (`kal/contrat.h`) et le dispatcher
(`kal.h`) y sont documentés.
