# CLAUDE.md — Lepton RTOS : migration IAR/Windows → GCC/Linux

<!-- Provenance : généré depuis le plan de migration (doc/migration/) via le skill
     lepton-portage-instructions, complété aux étapes 1 à 7. État final au 2026-10-06. -->

## 1. Objectif  [HUMAIN]

Lepton est un RTOS embarqué en C (KAL + noyau POSIX 1003.1 + VFS + réseau, ~1 MLOC).
**Migration terminée** (étapes 0 à 7 validées, la dernière le 2026-10-06 ; publiée sur
`master`) : build IAR/Windows remplacé par CMake + GCC sous Debian (hôte Debian natif), conduit
par `doc/migration/ORCHESTRATION.md`. IAR n'est ni conservé ni utilisé comme référence ; la
validation se fait par exécution (QEMU et cartes). La version IAR publiée avant la migration est
conservée : tag `version-4.10.0.2`, branche `lts/4.10.0.2`.

**Hors périmètre :** le code gelé (ARM7, ARM9, M16C, simulations Linux et Windows, cartes
abandonnées — `doc/migration/code-gele.md`, supprimé à l'étape 6, tag local `legacy`) ; le
Cortex-M3 (`mps2-an385`, décision 2026-10-05) ; le portage RISC-V (reporté, annexe) ; toute
refactorisation non demandée ; toute fonctionnalité hors du plan.

---

## 2. Contraintes non négociables  [HUMAIN]

| Élément | Valeur imposée |
|---|---|
| Cibles | QEMU `mps2-an386` (M4F, socle) et `mps2-an500` (M7) ; NUCLEO-F439ZI (base, validée sur NUCLEO-F429ZI), STM32F746G-DISCO (M7 r0p1), NUCLEO-WL55JC1 (M4 sans FPU), SAMD21 Xplained Pro (M0+) |
| Toolchain cible | `arm-none-eabi-gcc` (mono-toolchain GCC) ; `-Os -g` (`LEPTON_OPT_LEVEL`) |
| Micro-noyau | **deux backends maintenus** (décision 2026-10-06) : embOS port GCC Segger 5.20 et FreeRTOS 202604 LTS (noyau V11.3.0) ; SAMD21 : système complet sous embOS seulement |
| Architecture | quatre axes ISA / cœur / carte / micro-noyau ; un nouveau cœur = fichiers nouveaux aux emplacements de `doc/migration/ajout-coeur.md` |
| Build system | CMake ≥ 3.24 + presets `<machine>-<micro-noyau>[-soft]` ; bibliothèques statiques par composant (`bin`, `sbin`, `lib` hors de `kernel/`) ; mklepton natif Linux (génération C + rootfs, sorties par `--output-dir` dans `$LEPTON_BUILD`) |
| Environnement de build | Debian natif (`scripts/install-debian.sh --with-debug-tools`) ; conteneur `ci/Dockerfile` écrit, jamais construit (CI validée en natif) |
| Arbre des sources | composé par `scion` 0.5.0.1 (rootstock, seed `lepton-seed.scions` branche `original-tree`) : `trunk/` = liens relatifs vers `depots/` (ETAPE-0) |
| Versions | épinglées dans `doc/migration/MIGRATION-STATUS.md` |

**Interdits explicites :**
- **Aucune opération git distante sans accord explicite de l'utilisateur**, demandé à chaque
  fois (branche et commits concernés) : pas de `git push`, pas de création de branche ou de PR
  distante. `origin` est en SSH ; pousser des références nommées seulement, jamais `--tags` ni
  `--all` : les tags locaux `legacy` et `legacy-iar` ne doivent jamais être publiés.
- Ne pas modifier les paquets vendored (`third_party/embos/...`, noyaux FreeRTOS sous `ucore/`).
- Ne pas casser le socle QEMU ni la matrice micro-noyau × machine (`ci/run.sh` vert avant
  chaque commit).
- Aucune branche IAR dans le code (`compiler.h` : GCC seul).
- Aucune modification de masse à la main : scripts de transformation rejouables uniquement.
- Ne pas assouplir un critère de validation ni désactiver un test pour « faire passer ».
- **Ne jamais éditer ni créer de fichier dans `trunk/`** : éditer dans le clone
  `depots/lepton/original/master/scion/…` puis `scion graft`. `sed -i` sans
  `--follow-symlinks` remplace un lien par un fichier régulier non versionné.
- Pas de répertoire de build ni de sortie mklepton dans `trunk/` (tout va dans `$LEPTON_BUILD`).
- Pas de `scion seed-update` / `scion graft-update` (git pull) sans décision humaine.
- Aucun flag spécifique compilateur hors de `cmake/` ; aucun `#ifdef` d'ISA ou de cœur hors
  de `kal/arch/` et des répertoires d'architecture ; aucune adresse de carte hors de
  `cmake/boards/`, `ld/mem_*` et du BSP.

---

## 3. Commandes du projet  [AGENT OK]

```bash
source scripts/lepton-env.sh     # LEPTON_ROOTSTOCK, LEPTON_TRUNK, LEPTON_CLONE, LEPTON_BUILD
scion rootstock-information      # rootstock et scions greffés
scion graft                      # regreffe après ajout de fichier dans le clone
git status                       # git natif (cwd = racine du clone) ; jamais `scion git`
cd "$LEPTON_TRUNK" && cmake --preset <preset> && cmake --build --preset <preset>
ctest --preset <preset> -L host|smoke|net|kal   # hôte, fumée et réseau QEMU, banc KAL
ctest --preset <carte>-<µn> -L board|radio      # sur carte (sonde) ; -DLEPTON_BOARD_SERIAL_PORT=…
cmake --build --preset <carte>-<µn> --target flash   # reflasher lepton.elf après le banc KAL
tests/endurance_board.py …       # endurance sur carte (palier 8)
ci/run.sh                        # non-régression complète (matrice micro-noyau × machine)
python3 tools/migration/audit_iar.py           # compteur d'IAR-ismes (zéro sur le périmètre actif)
tools/migration/mass_compile.sh                # compilation de masse GCC
# Flash/debug : OpenOCD sur cette machine (USB direct), debug/openocd-*.cfg, debug/gdbinit-*
# (lepton-stacks, lepton-fault) ; procédure doc/migration/debug-gcc.md
# Fin de session : commit LOCAL ; jamais de push sans accord explicite
export PYTHONDONTWRITEBYTECODE=1 # avant tout ctest hors ci/run.sh (pas de __pycache__ dans le trunk)
```
Presets : `host` ; QEMU `qemu-mps2-an386-{embos,freertos}[-soft]`, `qemu-mps2-an500-{embos,freertos}` ;
cartes `{nucleo-f439zi,stm32f746g-disco,nucleo-wl55jc1,samd21-xplained-pro}-{embos,freertos}`.

---

## 4. Structure du dépôt  [AGENT OK]

```
<ROOTSTOCK>/depots/lepton/original/master/   clone git de l'arbre Lepton (où l'on édite)
<ROOTSTOCK>/trunk/                           arbre composé (liens), vue de build
  sys/root/src/kernel/{arch,core,dev,fs,net}  noyau ; core/kal.h (dispatcher), core/kal/{arch,backend}
  core/core-{segger,freertos,static,generic}  noyau Lepton par micro-noyau
  cmake/{isa,cpu,boards,kal,components}       axes du build ; ld/ (mem_<carte>.ld, common-cortexm.ld)
  tests/, debug/                              tests (host, QEMU, carte, banc KAL), OpenOCD et gdb
  tools/{mklepton,host,...}                   outils hôte
  building/{projects,staging,output}          emplacement de génération (scion building)
<ROOTSTOCK>/third_party/embos/               paquet embOS GCC (licence SFL), hors dépôt
<ROOTSTOCK>/build/                           $LEPTON_BUILD : un répertoire par preset, ci/
```
Dépôt `lepton-original-tree.scions` (le clone, répertoire de lancement) :
- racine, **non greffée** : `CLAUDE.md`, `doc/migration/`, `scripts/`, `.claude/`,
  `tools/migration/` (audit, transformation, métriques), `ci/` ;
- sous `scion/`, **greffé** (vu dans le trunk) : `CMakeLists.txt`, `CMakePresets.json`, `cmake/`,
  `ld/`, `tests/`, `debug/`, et les sources Lepton.
Les chemins `src/…` du plan sont relatifs à `sys/root/`. Les projets IAR (`sys/root/prj/`,
`.ewp`, `.eww`, `.icf`) ont été supprimés à l'étape 6 (tag local `legacy-iar`).

Règles de l'arbre composé (ETAPE-0 tâche 5 ; rootstock `~/lepton`, trunk `trunk/`) :

| Sujet | Règle |
|---|---|
| Lecture, build | Par le trunk (aucun chemin `depots/` codé en dur dans l'arbre Lepton). |
| Écriture | Dans le clone `depots/lepton/original/master/scion/…`, puis `scion graft`. Jamais dans le trunk. |
| Git | Natif dans le clone (`git -C "$LEPTON_CLONE"`) ; le trunk n'est pas un dépôt. |
| Build | `$LEPTON_BUILD` (`<ROOTSTOCK>/build/`), hors du trunk et hors du clone. |
| Sorties de mklepton | `$LEPTON_BUILD/<preset>/generated/` (option `--output-dir`, décision 2026-09-30). |
| Conteneur | Monter le rootstock entier : les liens sont relatifs à lui. |
| Mise à jour des clones | `scion seed-update` / `graft-update` (git pull) : décision humaine. |
| `.gitignore` du clone | Hérité de Visual Studio/IAR : ignore notamment `[Dd]ebug/`, `[B]in/`, `*.bin`, `*.a`, `[Ss]ettings/`. Vérifier `git status --ignored` avant chaque commit ajoutant des fichiers. |
| Hook `lepton_guard.py` | Résout les chemins depuis le répertoire courant de la session et lit tout `>` d'un argument comme une redirection : scripts dans le scratchpad, messages de commit par `git commit -F`. |
| Encodage | Sources Lepton souvent en Latin-1 : éditer par script (`encoding="latin-1"`) ; `grep` de l'hôte = `ugrep` (`grep -a`, `grep -R` pour suivre les liens du trunk). |

---

## 5. Conventions  [HUMAIN + AGENT]

- Langue : français (commentaires migration, docs) ; ton neutre et concis.
- Abstractions : extensions compilateur via `compiler.h` (macros `__lepton_*`) uniquement.
- Commits : mécaniques (un par répertoire × règle, message citant le script) strictement
  séparés des sémantiques (petits, relus). Une branche locale par chantier
  (`migration/etape-N` pendant la migration), fusionnée dans `master` après validation.
- Sorties volumineuses (audits, maps, inventaires) dans `doc/migration/`, jamais en
  conversation ni recopiées ici.

---

## 6. Méthode de travail  [HUMAIN]

### 6.1 Point d'entrée
Le plan séquentiel (étapes 0 à 7) est terminé. Tout nouveau chantier (dette de
`MIGRATION-STATUS.md`, annexe RISC-V, nouvelle carte) commence par un fichier d'instructions
produit avec le skill `lepton-portage-instructions`, puis suit `doc/migration/ORCHESTRATION.md`
(lire `MIGRATION-STATUS.md`, mode plan, exécuter, valider, clore). Sessions lancées par
`scripts/claude-lepton.sh`, à la racine du clone.

### 6.2 Règle anti-invention
Toute information manquante (offset, format, convention d'empilement, version) doit être
cherchée dans le code, la doc du port livré, ou les rapports d'audit. Introuvable →
question à l'utilisateur ; hypothèse indispensable → marquer `HYPOTHÈSE À VALIDER :` dans
le code et la lister au compte rendu. Jamais de supposition silencieuse.

### 6.3 Points d'arrêt
S'arrêter et faire valider : chaque décision du tableau §4 d'`ORCHESTRATION.md` ; fin de
chaque étape ou chantier même si tout est vert ; avant toute opération irréversible (écriture
flash, suppression de code) ; après trois échecs successifs sur un même point.

### 6.4 Comptes rendus
Clore chaque session par : fait / **vérifié et comment** / incertain / prochaine action,
la mise à jour de `MIGRATION-STATUS.md` et du handoff (`doc/migration/handoff/`, ORCHESTRATION §3bis). Ne pas présenter comme validé ce qui n'a été
que compilé.

---

## 7. Zones sensibles  [HUMAIN]

| Chemin / ressource | Régime |
|---|---|
| `third_party/`, noyaux vendored (`ucore/freeRTOS_*`, `ucore/cmsis*`, HAL ST) | Lecture seule |
| Licence embOS (SFL : évaluation / non commercial, redistribution interdite) | Paquet hors dépôt ; réévaluation avant tout usage produit |
| `compiler.h`, CMake communs, `MIGRATION-STATUS.md` | Session principale uniquement (jamais un sous-agent) |
| `trunk/` | Lecture seule — vue composée ; toute écriture passe par le clone. Bloqué par le hook `.claude/hooks/lepton_guard.py` |
| `depots/lepton-seed.scions/`, `depots/generation/` | Lecture seule — seed et emplacement gérés par scion |
| Dépôt `seed.scions` (outil scion) | Hors périmètre — projet distinct ; défauts signalés par issue, jamais corrigés ici |
| `scion seed-update`, `graft-update`, `graft-clean` | Point d'arrêt — validation explicite requise |
| Écriture flash sur cible | Validation explicite requise |
| Tags locaux `legacy`, `legacy-iar` (code gelé et IAR supprimés) | Ne jamais pousser |
| périmètre actif hors fichiers du chantier en cours | Point d'arrêt avant modification |

---

## 8. Validation  [HUMAIN]

**Bancs :** QEMU `mps2-an386` et `mps2-an500` (UART CMSDK, Ethernet LAN9118) : fumée
(`smoke_lsh.py` : démarrage → `lsh` → `uname -a`), réseau (ping, `ftpd`) et banc KAL
(`doc/migration/BANC-TEST-KAL-QEMU.md`) ; cartes via sonde USB : `ctest -L board` (fumée et
banc KAL par semihosting), `board.net`, `board.radio`, endurance (`tests/endurance_board.py`).

**Oracle :** aucun binaire IAR. Oracles : exécution (paliers des étapes 3 et 5), sorties de
référence de mklepton (`mklepton-ref.md`), comparaison embOS / FreeRTOS test par test.

**Critères d'acceptation permanents :**
- [x] Socle QEMU vert à chaque commit (`ci/run.sh` : 6 presets QEMU testés, 8 presets carte construits).
- [x] `audit_iar.py` à zéro sur le périmètre actif (étape 6).
- [x] Critères des fichiers `ETAPE-0` à `ETAPE-7` cochés ; écarts acceptés listés dans
  `MIGRATION-STATUS.md` et `handoff/etape-7.md`.

---

## 9. Points ouverts  [AGENT OK]

<!-- Liste vivante — reprendre aussi le tableau des décisions d'ORCHESTRATION.md §4. -->
Tranchés pendant la migration :
- [x] BSP embOS le plus proche du F439 : `ST/STM32F429_STM32F429ZI_Nucleo`, consulté seulement
  (horloge) ; intégration écrite pour Lepton.
- [x] Discovery F7 : STM32F746G-DISCO (étape 6).
- [x] Noyau statique : second pilote logiciel `dev_part` ; image UFS sur un pilote bloc fichier
  hôte (`kernel/dev/arch/host/`) (étape 2).
- [x] Contenu de `bin` : tests POSIX T9-T11 du banc KAL (étape 3).
- [x] mklepton : sources trouvées, portage natif (pas de repli) ; oracle = sorties versionnées.
- [x] Licence embOS : SEGGER Friendly License, évaluation / non commercial.
- [x] Trunk : chemins mkconf relatifs ; rootstock `~/lepton`, trunk `trunk/` ; Claude Code lancé
  à la racine du clone par `scripts/claude-lepton.sh` (décision 2026-09-30).
- [x] Devenir d'embOS après FreeRTOS : maintenu (décision 2026-10-06).

Ouverts (décision humaine ou chantier à planifier) :
- [ ] Portage RISC-V (`ANNEXE-nouveau-coeur-riscv.md`) : reporté.
- [ ] SAMD21 sous FreeRTOS : système complet non supporté (RAM), écart accepté ; piste :
  sémaphore `core-freertos` plus léger.
- [ ] Conteneur `ci/Dockerfile` jamais construit.
- [ ] Chantier « hôte macOS » (ouvert le 2026-10-07) : `ETAPE-8-hote-64-bits.md` (Debian),
  `ETAPE-9-hote-macos.md`, `ETAPE-10-cartes-macos.md` ; état dans `MIGRATION-STATUS.md`.
- [ ] Dette technique : section « Blocages et dette » de `MIGRATION-STATUS.md`.

---

## 10. Documentation de référence

| Fichier | Contenu |
|---|---|
| `doc/migration/README.md` | plan global, environnement hôte, graphe des étapes |
| `doc/migration/ORCHESTRATION.md` | machine à états, décisions humaines, discipline git, agents |
| `doc/migration/MIGRATION-STATUS.md` | avancement, matrices, versions épinglées, décisions, dette |
| `doc/migration/handoff/etape-7.md` | état final : critères, écarts acceptés, empreintes embOS/FreeRTOS |
| `doc/migration/ETAPE-0..7, BANC-TEST-KAL-QEMU, ANNEXE-nouveau-coeur-riscv` | instructions d'étape |
| `doc/migration/validation-*.md` | journaux de validation par machine (paliers, défauts, mesures) |
| `doc/migration/kal-freertos-ecarts.md` | écarts KAL embOS → FreeRTOS, accès internes |
| `doc/migration/ajout-coeur.md` | procédure d'ajout d'un cœur ou d'une carte |
| `doc/migration/debug-gcc.md` | procédure de débogage OpenOCD/gdb |
| `doc/migration/sources/lepton-migration-guide-step-1.md` | guide de l'auteur : noyau statique, mklepton, noyau dynamique |
| README de `lepton-distribution/seed.scions` (tag 0.5.0.1) | commandes, formats `.scion.*`, layout rootstock |
| `seed.scions/doc/migration/MIGRATION-STATUS.md` | document du projet scion (externe) ; constats sur l'arbre Lepton repris dans `doc/migration/MIGRATION-STATUS.md` |
