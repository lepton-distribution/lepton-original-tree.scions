# Étape 1 — Inventaire de l'arbre et délimitation du périmètre

## Contexte

Lepton est un RTOS embarqué en C (KAL + noyau POSIX + VFS + réseau, ~1 MLOC), aujourd'hui construit
par des projets IAR `.ewp` sous Windows. Cible de la migration : Linux Debian, CMake,
`arm-none-eabi-gcc`. **IAR n'est ni conservé ni utilisé comme référence** : aucune comparaison
binaire avec IAR n'est faite ; les `.ewp` ne servent qu'à connaître les listes de sources, includes
et defines. La validation se fait par exécution : QEMU (`mps2-an386`, étape 3), puis la carte
NUCLEO-F439ZI (étape 5). La simulation Linux est abandonnée. Cette étape ne modifie aucun fichier
source : elle produit l'état des lieux qui pilote toutes les suivantes.

## Prérequis

- Étape 0 terminée : rootstock scion en place, clone `depots/lepton/original/master`, trunk greffé,
  handoff `handoff/etape-0.md`.
- Les audits lisent le trunk avec `find -L` ou directement le clone (`scion/…`) ; les chemins des
  rapports sont exprimés relativement à `sys/root/` ou `tools/`.

## Tâches

### 0. Paquets embOS (Cortex-M GCC et RISC-V, téléchargés)

- Relever la version exacte de chaque paquet ; inventorier le contenu (bibliothèques `.a`, `RTOS.h`,
  RTOSInit, handlers PendSV/SysTick en `.S` GNU, exemples de `.ld`/startup) : ce que Segger fournit
  n'est ni écrit ni traduit.
- Vérifier les variantes de bibliothèques : ARMv6-M (M0/M0+), M3, M4/M7 avec et sans FPU
  (hard-float). Noter la correspondance variante ↔ flags pour `cmake/cpu/*.cmake` (étape 2).
- Vérifier que le BSP ou l'exemple le plus proche du **STM32F439** (ou F429/F4) figure dans le paquet.
- Relever les conditions de licence (évaluation/production, redistribution) ; intégrer les paquets
  dans `third_party/embos/<famille>/<version>/`, hors audit.
- Extraire l'API publique des headers (préparation de la tâche 4).

### 1. Projets IAR : extraction des listes de sources

- Lister `.eww`/`.ewp`/`.ewd` (46 `.ewp` relevés à l'étape 0, sous `sys/root/prj/iar/`, de plusieurs
  générations d'EWARM jusqu'à 8.40).
- Pour chaque `.ewp`, extraire par script (`tools/migration/ewp_extract.py`) : sources C/asm,
  includes, defines par configuration, options cpu/fpu, `.icf` référencé, étapes custom (mklepton).
- Produire `doc/migration/inventaire-projets.md` : un tableau par projet, en signalant les projets
  STM32F4 (`dev/stm32f4xx`, `bsp/discovery_f4`, `bsp/olimex_p407`, `bsp/stm32f469i-eval`) qui
  serviront de base à la liste de sources de la NUCLEO-F439ZI, et ceux des cartes de l'étape 6.

### 2. Délimitation du périmètre (~1 MLOC)

- Construire la matrice fichier × projet (`tools/migration/build_closure.py`, CSV).
- Trois ensembles, chiffrés par répertoire (`cloc`) :
  - **Actif** : fermeture du socle Cortex-M (noyau, VFS, POSIX, réseau, `lsh`, KAL `core-segger`,
    code Cortex-M commun) plus le BSP STM32F4 ; seul code traité aux étapes 3 à 5 ;
  - **Différé** : propre aux autres cartes retenues (étape 6) ;
  - **Gelé** : ARM7, ARM9, M16C, simulations (`dev/arch/gnu32`, `dev/arch/win32`, `core/arch/win32`,
    `tools/virtual_cpu`, `prj/vc-2010`), cartes abandonnées — listé dans `doc/migration/code-gele.md`,
    décision soumise à l'utilisateur. `prj/scons` n'est pas gelé d'office : il contient le build de
    l'ancien noyau statique utilisé par mklepton (tâche 5).
- Pas de projet IAR pour `mps2-an386` ni pour la NUCLEO-F439ZI : leur liste de sources est dérivée
  du socle commun + projet STM32F4 le plus proche ; consigner la dérivation.

### 3. Audit des IAR-ismes

Script rejouable `tools/migration/audit_iar.py` (fichier:ligne, ventilé actif/différé/gelé) :
mots-clés étendus (`__no_init`, `__root`, `__ramfunc`, `__weak`, `__packed`, `__irq`, `__swi`,
`__intrinsic`, `__noreturn`, placement `@`), pragmas (`location`, `section`, `optimize`,
`data_alignment`, `vector`, `type_attribute`, `object_attribute`), intrinsics (`__enable_interrupt`,
`__disable_interrupt`, `__get_*`/`__set_*`, `<intrinsics.h>`), headers DLib/`<yfuns.h>`, fichiers
asm (`.s`, `.s79`, `.asm` avec leur rôle), gardes `__compiler_*`/`__ICCARM__`/`__GNUC__`.
Sortie : `doc/migration/audit-iar.md` + `audit-iar.csv`.

### 4. KAL, backends et code dépendant du cœur

- KAL : `src/kernel/core/kal.h` ; backends `core-segger`, `core-freertos`, `core-generic` (relevés à
  l'étape 0). Décrire le rôle réel de chacun ; `core-freertos` existant sera la base de l'étape 7.
- Écart embOS IAR vs GCC : champs de TCB et fonctions d'API utilisés par le KAL, comparés aux
  headers de la tâche 0 → `doc/migration/embos-iar-vs-gcc.md` avec chiffrage.
- Localiser et classer par **axe de variation** (préparation de l'architecture multi-cœurs de
  l'étape 2) : ce qui dépend de l'ISA (Thumb/ARMv7-M : SVC, contexte, startup), du cœur (FPU M4F/M7,
  cache M7, ARMv6-M pour M0), de la carte (horloges, UART, Ethernet, mémoire), du micro-noyau. Pour
  chaque élément : appartient à embOS (fourni par Segger) ou à Lepton (à porter).
- Documenter dans `doc/migration/cartographie-kal.md`.

### 5. mklepton et noyau statique hôte

- mklepton se lie à une bibliothèque noyau Lepton compilée pour l'hôte (`-lkernel`,
  `tools/mklepton/prj/scons/SConstruct`), construite par
  `prj/scons/arch/synthetic/x86_static/SConstruct`. Relever cette liste de sources et ses écarts avec
  l'arbre actuel (déjà constatés : `kernel/core/core-ecos/` et `kernel/core/arch/synthetic/x86_static/`
  absents ; pilotes `gnu32` gelés).
- Cibler le contenu prévu par le guide de l'auteur (`sources/lepton-migration-guide-step-1.md`,
  §1.1) : `kernel/core` sans ordonnanceur, pilotes logiciels `dev_cpufs`, `dev_head`, `dev_null`,
  `dev_proc`, `dev_tty`, `fs/vfs`, `fs/rootfs`, `fs/ufs`. Lister, pour ces fichiers, les
  dépendances à l'ordonnanceur et aux appels système à couper.
- Relever les structures écrites dans l'image UFS et la taille de leurs types (préparation de la
  vérification de format hôte / ARM de l'étape 2).
- Sources `tools/mklepton/src/`, binaire Linux i386 `tools/bin/mklepton_gnu` et son script ;
  entrées (XML `mkconf_*`, contenu du rootfs) et sorties exactes (C généré, système de fichiers flash,
  emplacements `dest_path`).
- Constituer un **jeu de sorties de référence** avec le binaire existant (`mklepton_gnu`, ou
  `mklepton.exe` si disponible) : oracle du portage natif de l'étape 2.

### 6. Graphe des dépendances entre composants

- Compiler chaque répertoire séparément pour l'hôte (`gcc -c`, sans édition de liens) et extraire
  symboles définis et non résolus (`nm`) ; en déduire le graphe orienté entre composants :
  `kernel/core`, `kernel/core/core-segger`, `kal`, `kernel/dev` (pilotes logiciels et matériels
  séparés), `kernel/fs/vfs`, chaque système de fichiers, `lib/libc`, `lib/pthread`, `sbin`, `bin`.
- Signaler les cycles et leur densité (nombre de symboles en jeu) ; script
  `tools/migration/dep_graph.py`, sortie `doc/migration/dependances.md` (+ `.dot`).
- Ce graphe fixe le découpage en bibliothèques de l'étape 2.

### 7. Graphe Graphify (optionnel)

Sur le clone (`depots/lepton/original/master/scion`), après la tâche 2 ; exposé en MCP. Aide à
l'exploration, jamais source d'autorité ; relations macro-générées incomplètes.

## Critères de validation

- [ ] Paquets embOS inventoriés, variantes et licence consignées, BSP le plus proche du F439 identifié.
- [ ] `inventaire-projets.md`, `audit-iar.md`, `cartographie-kal.md`, `embos-iar-vs-gcc.md` produits.
- [ ] Matrice fichier × projet ; volumétrie des trois ensembles ; `code-gele.md` soumis à l'utilisateur.
- [ ] Classement par axe (ISA / cœur / carte / micro-noyau) de tout le code dépendant du matériel.
- [ ] Graphe des dépendances entre composants, cycles identifiés (`dependances.md`).
- [ ] Jeu de sorties de référence mklepton archivé ; inventaire du noyau statique (sources,
      dépendances à couper, structures de l'image UFS) produit.
- [ ] Aucun fichier source modifié (`git -C "$LEPTON_CLONE" status` propre) ; aucun
      fichier régulier dans le trunk.

## Pièges connus

- `.ewp` : chemins Windows (`..\`), variables (`$PROJ_DIR$`, `$TOOLKIT_DIR$`), fichiers exclus par
  configuration — normaliser et en tenir compte.
- Formats `.ewp` différents selon la génération d'EWARM (`fileVersion` 1 à 4) : tester le script sur
  chaque génération.
- Des IAR-ismes peuvent venir des modèles utilisés par mklepton : auditer aussi les XML.

## À la fin de l'étape

Mettre à jour `MIGRATION-STATUS.md` (volumétrie, comptes d'IAR-ismes, liste de sources dérivée
pour `mps2-an386` et F439) et produire `handoff/etape-1.md`.
