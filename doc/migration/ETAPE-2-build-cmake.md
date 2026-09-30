# Étape 2 — Système de build CMake, noyau statique hôte, mklepton natif

<!-- Sources : sources/lepton-migration-guide-step-1.md (auteur, §1.1 et §1.2) ; relevés de
     l'arbre (tools/mklepton/prj/scons/SConstruct, prj/scons/arch/synthetic/x86_static/SConstruct). -->

## Contexte

Cette étape pose le système de build définitif (CMake, architecture ouverte à de nouveaux cœurs) et
produit tout ce qui s'exécute **sur l'hôte Linux** : le noyau Lepton statique, puis mklepton.
mklepton n'est pas un outil autonome : il se lie à une bibliothèque noyau Lepton compilée pour
l'hôte (`-lkernel`), qu'il utilise, sans ordonnanceur ni appels système, pour construire le système
de fichiers UFS placé sur le périphérique `dev_cpufs` et monté au démarrage de la cible. Rien ne
s'exécute encore sur ARM : la chaîne croisée arrive à l'étape 3.

## Prérequis

- Étape 1 : inventaire, classement par axe, inventaire du noyau statique et de mklepton, jeu de
  sorties de référence mklepton.
- Décision de l'étape 0 sur le trunk (`tauon` dans `$HOME` ou chemins mkconf relatifs).
- CMake ≥ 3.24 (`$<LINK_GROUP:RESCAN,…>`), `ninja`, `gcc` hôte, `libexpat1-dev`.
- Graphe des dépendances entre composants (étape 1, tâche 6).

## Tâches

### 1. Architecture de build à quatre axes

Chaque axe a ses emplacements ; ajouter une valeur = ajouter des fichiers, sans modifier le code
commun (hors une ligne d'enregistrement).

| Axe | Valeurs (actuelles → futures) | Porté par |
|---|---|---|
| ISA / famille | `host` (noyau statique), `armv7m`, `armv6m` → `rv32` | `cmake/toolchains/`, asm de démarrage et d'appel système, `kal/arch/<famille>/` |
| Cœur | `cortex-m0plus`, `cortex-m3`, `cortex-m4f`, `cortex-m7` → `rv32imac` | `cmake/cpu/<cœur>.cmake` |
| Carte | `qemu-mps2-an386`, `nucleo-f439zi` → autres | `cmake/boards/<carte>.cmake`, `ld/mem_<carte>.ld`, BSP |
| Micro-noyau | aucun (statique), `embos` → `freertos` | `LEPTON_KAL_BACKEND`, `kal/backend/<rtos>/` |

- Fichiers CMake sous `scion/` (greffés) : `CMakeLists.txt`, `CMakePresets.json`,
  `cmake/{toolchains,cpu,boards}/`, `cmake/mklepton.cmake`, `cmake/lepton_target.cmake`, `ld/`.
- Presets : `binaryDir` = `${sourceDir}/../build/${presetName}`, soit `$LEPTON_BUILD/<preset>` —
  jamais dans le trunk (un fichier régulier y bloque `scion graft`) ni dans le clone. Les commandes
  CMake se lancent depuis le trunk (`cd "$LEPTON_TRUNK" && cmake --preset …`).
- Deux familles de presets : **hôte** (noyau statique, mklepton, tests hôte) et **croisés** (cartes).
  Le build d'une carte dépend du build hôte (mklepton) : superbuild ou `ExternalProject`.
- Sélection par `LEPTON_ISA`, `LEPTON_CPU`, `LEPTON_BOARD`, `LEPTON_KAL_BACKEND`, jamais par
  macros du compilateur.
- Aucun flag compilateur hors de `cmake/` ; emplacements des sources d'architecture alignés sur
  l'arborescence réelle et consignés dans `doc/migration/ajout-coeur.md` (liste des fichiers à
  créer pour un nouveau cœur, une nouvelle famille, une nouvelle carte).

### 2. Bibliothèques par composant

Bibliothèques **statiques** (chemins relatifs à `sys/root/src/` ; `bin`, `sbin` et `lib` restent
hors de `kernel/`), communes au noyau statique hôte et au noyau dynamique :

| Bibliothèque | Contenu |
|---|---|
| `lepton_core` | `kernel/core`, partie commune ; compilée sans ordonnanceur (noyau statique) ou complète (dynamique) |
| `lepton_kal_<rtos>` | `kernel/core/core-<rtos>` et `kal` ; absente du noyau statique |
| `lepton_dev` | pilotes logiciels de `kernel/dev` (`dev_cpufs`, `dev_head`, `dev_null`, `dev_proc`, `dev_tty`…) |
| `lepton_bsp_<carte>` | pilotes matériels de la carte (jamais dans `lepton_dev`) |
| `lepton_vfs` | `kernel/fs/vfs` |
| `lepton_fs_<fs>` | un système de fichiers par bibliothèque : `rootfs`, `ufs` ; plus tard `fat`, `kofs`, `yaffs` |
| `lepton_libc`, `lepton_pthread` | `lib/libc`, `lib/pthread` |
| `lepton_sbin`, `lepton_bin` | `sbin`, `bin` ; statiques pour que l'éditeur de liens n'extraie que les pseudo-binaires référencés par la table générée par mklepton (`bin_mkconf.c`) |

- Édition de liens finale en groupe (`$<LINK_GROUP:RESCAN,…>`) : le noyau, le VFS et les pilotes
  s'appellent mutuellement (montages au démarrage, tables de pilotes générées, rappels du noyau).
- Si le graphe de l'étape 1 montre un cycle serré entre deux composants, les fusionner plutôt que
  de le contourner ; consigner la décision dans `ajout-coeur.md`.
- Chaque bibliothèque déclare ses dépendances (`target_link_libraries`) conformément au graphe.

### 3. Noyau Lepton statique pour l'hôte (guide §1.1)

Bibliothèque `libkernel` pour Linux, sans ordonnanceur ; les fonctions qui en dépendent ne sont pas
compilées. Contenu, dans cet ordre :

1. `kernel/core` : base du noyau ;
2. `kernel/dev` : pilotes logiciels seulement — `dev_cpufs`, `dev_head`, `dev_null`, `dev_proc`,
   `dev_tty` (+ un pilote <À CONFIRMER : `dev_null` est cité deux fois dans le guide>) ;
3. `kernel/fs` : `vfs`, puis `rootfs` (en RAM, sans pilote), puis `ufs` ;
4. `lib` (`libc`, `pthread`) : seulement ce que les trois premiers exigent (établi à l'étape 1 ;
   l'ancien noyau statique les incluait).

(Chemins relatifs à `sys/root/src/`.)

- Point de départ : la liste de l'ancien build `prj/scons/arch/synthetic/x86_static/SConstruct`.
  Elle est **obsolète** : elle référence `kernel/core/core-ecos/` et
  `kernel/core/arch/synthetic/x86_static/`, absents de l'arbre, ainsi que des pilotes `gnu32`
  (code gelé). La reconstruire, ne pas la réactiver.
- Configuration propre au noyau statique (équivalents de `kernelconf.h`, `dev_mkconf.c`,
  `bin_mkconf.c` pour l'hôte) : fixe et minimale, écrite à la main (elle ne peut pas venir de
  mklepton, qui dépend de cette bibliothèque).
- **Point d'arrêt — stockage de l'image** : l'ancien build adossait l'image à un fichier hôte
  (`dev_linux_filerom`, `dev_linux_fileflash`, code gelé). Faire trancher le mécanisme retenu
  (image construite en RAM sur `dev_cpufs` puis écrite dans un fichier, ou pilote fichier hôte
  réintroduit).
- **Compatibilité du format** : l'image UFS produite sur l'hôte est lue par une cible ARM 32 bits.
  Vérifier que toutes les structures écrites sur le support ont la même taille et le même
  alignement sur l'hôte et sur `arm-none-eabi` (assertions statiques `sizeof`/`offsetof` compilées
  des deux côtés). Si ce n'est pas le cas : **point d'arrêt** (corriger les types, ou compiler le
  noyau statique en 32 bits).
- Tests hôte : programme de test qui monte `rootfs`, crée et remplit un UFS, le relit ; exécuté par
  CTest (label `host`). C'est le premier banc qui exerce VFS et UFS, avant QEMU.

### 4. mklepton natif (guide §1.2)

- Sources `tools/mklepton/src/` (`mklepton.c`, `mklepton.h`, `kernel_stub.h` ; `mklepton-w32.c`
  écarté) ; dépendances : expat, `libkernel` de la tâche 2.
- Existant : `tools/bin/mklepton_gnu` (ELF i386, oracle) et `mklepton_gnu.sh` (remplace `$(HOME)`
  par `sed`) ; mkconf avec `dest_path="$(HOME)/tauon/…"`.
- **Point d'arrêt — sorties** : les mkconf écrivent dans le trunk (à travers un lien, ils modifient
  un fichier versionné ; en fichier régulier, ils bloquent `scion graft`). Faire trancher : option de
  sortie vers le répertoire de build, ou `dest_path` hors du trunk.
- Oracle : sorties (C généré, image UFS) identiques octet à octet au jeu de référence de l'étape 1,
  pour les mêmes XML.
- `cmake/mklepton.cmake` : `lepton_generate(<cible> XML <fichier>)`, `add_custom_command` avec
  `DEPENDS` sur le XML, le contenu du rootfs et l'exécutable ; sorties dans le répertoire de build.

## Critères de validation

- [ ] Preset hôte : `libkernel` (assemblage des bibliothèques de la tâche 2) et `mklepton`
      construits ; `ctest -L host` vert.
- [ ] Dépendances entre bibliothèques conformes au graphe de l'étape 1 ; cycles résolus par
      groupe d'édition de liens ou fusion documentée.
- [ ] Assertions de format UFS identiques hôte / `arm-none-eabi` (compilation des deux côtés).
- [ ] mklepton : sorties identiques octet à octet au jeu de référence ; aucune écriture dans le trunk.
- [ ] Presets croisés `qemu-mps2-an386-embos` et `nucleo-f439zi-embos` décrits (validés à l'étape 3).
- [ ] `ajout-coeur.md` rédigé ; `grep` : aucun `-mcpu`/`-mfpu` hors `cmake/`.
- [ ] Après un build complet, `find "$LEPTON_TRUNK" -type f ! -name .scion.grafted.list` vide.

## Pièges connus

- `mklepton_gnu` exige `libc6:i386` et d'être lancé depuis `tools/bin`.
- Le noyau statique ne doit rien tirer de l'ordonnanceur : une dépendance cachée se révèle à
  l'édition de liens ; la couper par la configuration, pas par des bouchons silencieux.
- Defines différents par configuration dans les `.ewp` : les porter par preset.

## À la fin de l'étape

`MIGRATION-STATUS.md` : décisions (stockage de l'image, sorties de mklepton, format), presets ;
`handoff/etape-2.md`.
