# Construire Lepton (GCC, Linux) et flasher la NUCLEO-F439ZI

Procédure de bout en bout sur un hôte Debian 13 x86_64 : arbre des sources, build, tests sous
QEMU, flash et test sur la carte de base. Versions épinglées : `doc/migration/MIGRATION-STATUS.md`.

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

Non-régression complète (hôte, QEMU hard et soft-float, réseau, banc KAL) : `ci/run.sh` depuis
le clone.

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
