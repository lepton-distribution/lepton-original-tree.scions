# CLAUDE.md — Lepton RTOS : migration IAR/Windows → GCC/Linux

<!-- Provenance : généré depuis le plan de migration (doc/migration/) via le skill
     lepton-portage-instructions. Premier jet à relire par le commanditaire.
     Sections [AGENT OK] à compléter par Claude Code à l'étape 1, puis relues. -->

## 1. Objectif  [HUMAIN]

Lepton est un RTOS embarqué en C (KAL + noyau POSIX 1003.1 + VFS + réseau, ~1 MLOC).
Chantier en cours : migration du build IAR/Windows vers CMake + GCC sous Debian
(hôte Debian natif), pilotée par `doc/migration/ORCHESTRATION.md`. IAR n'est ni conservé ni
utilisé comme référence ; la validation se fait par exécution (QEMU, puis NUCLEO-F439ZI).

**Hors périmètre :** le code gelé (ARM7, ARM9, M16C, simulations Linux et Windows, cartes
abandonnées — liste dans `doc/migration/code-gele.md` après l'étape 1) ; le portage RISC-V
(reporté, annexe) ; toute refactorisation non demandée ; toute fonctionnalité hors du plan.

---

## 2. Contraintes non négociables  [HUMAIN]

| Élément | Valeur imposée |
|---|---|
| Cibles | QEMU `mps2-an386` (socle), NUCLEO-F439ZI (base), puis M7, M3, M0/M0+ (étape 6) |
| Toolchain cible | `arm-none-eabi-gcc` (état final mono-toolchain GCC) |
| Micro-noyau | embOS port GCC Segger ; FreeRTOS à l'étape 7 |
| Architecture | quatre axes ISA / cœur / carte / micro-noyau ; un nouveau cœur = fichiers nouveaux aux emplacements de `doc/migration/ajout-coeur.md` |
| Build system | CMake ≥ 3.24 + presets ; bibliothèques statiques par composant (`bin`, `sbin`, `lib` hors de `kernel/`) ; mklepton natif Linux (génération C + rootfs flash, porté à l'étape 2) |
| Environnement de build | Debian natif (`scripts/install-debian.sh --with-debug-tools`) ; conteneur `ci/Dockerfile` = référence CI/reproductibilité |
| Arbre des sources | composé par `scion` 0.5.0.1 (rootstock, seed `lepton-seed.scions` branche `original-tree`) : `trunk/` = liens relatifs vers `depots/` (ETAPE-0) |
| Versions | épinglées dans `doc/migration/MIGRATION-STATUS.md` |

**Interdits explicites :**
- **Aucune opération git distante sans accord explicite de l'utilisateur**, demandé à chaque
  fois : pas de `git push`, pas de création de branche ou de PR distante. Toutes les opérations git
  sont locales (commit, branche, tag) jusqu'à nouvel ordre.
- Ne pas modifier le code gelé, ni les paquets vendored (`third_party/embos/...`).
- Dès l'étape 3, ne pas casser le socle QEMU (`ctest -L smoke`, `ctest -L kal`).
- Aucune branche IAR dans le code (`compiler.h` : GCC seul).
- Aucune modification de masse à la main : scripts de transformation rejouables uniquement.
- Ne pas assouplir un critère de validation ni désactiver un test pour « faire passer ».
- **Ne jamais éditer ni créer de fichier dans `trunk/`** : éditer dans le clone
  `depots/lepton/original/master/scion/…` puis `scion graft`. `sed -i` sans
  `--follow-symlinks` remplace un lien par un fichier régulier non versionné.
- Pas de répertoire de build ni de sortie mklepton dans `trunk/` (les mkconf actuels y écrivent
  via `$(HOME)/tauon/…` : conflit à trancher à l'étape 2, ne pas lancer mklepton avant).
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
ctest --preset <preset> -L host|smoke|kal       # tests hôte, fumée QEMU, banc KAL
ci/run.sh                        # non-régression complète (dès l'étape 3)
python3 tools/migration/audit_iar.py           # compteur d'IAR-ismes
tools/migration/mass_compile.sh                # compilation de masse GCC
# Flash/debug F439 : OpenOCD, sur cette machine (USB direct), voir debug/ et ETAPE-5
# Fin de session : commit LOCAL sur migration/etape-N ; jamais de push sans accord explicite
```
<À CONFIRMER : presets exacts après l'étape 2.>

---

## 4. Structure du dépôt  [AGENT OK]

```
<ROOTSTOCK>/depots/lepton/original/master/   clone git de l'arbre Lepton (où l'on édite)
<ROOTSTOCK>/trunk/                           arbre composé (liens), vue de build
  sys/root/src/kernel/{arch,core,dev,fs,net}  noyau ; core/kal.h, core/core-{segger,freertos,generic}
  sys/root/prj/{iar,scons,vc-2010,config}     projets existants (46 .ewp, lus puis supprimés à l'étape 6)
  tools/{mklepton,virtual_cpu,...}            outils hôte
  building/{projects,staging,output}          emplacement de génération (scion building)
```
Dépôt `lepton-original-tree.scions` (le clone, répertoire de lancement) :
- racine, **non greffée** : `CLAUDE.md`, `doc/migration/`, `scripts/`, `.claude/`,
  `tools/migration/` (audit, transformation, métriques), `ci/` ;
- sous `scion/`, **greffé** (vu dans le trunk) : `CMakeLists.txt`, `CMakePresets.json`, `cmake/`,
  `ld/`, `tests/`, et les sources Lepton.
Les chemins `src/…` du plan sont relatifs à `sys/root/`.

Règles de l'arbre composé (ETAPE-0 tâche 5 ; rootstock actuel `~/lepton`, trunk `trunk/`) :

| Sujet | Règle |
|---|---|
| Lecture, build | Par le trunk (aucun chemin `depots/` codé en dur dans l'arbre Lepton). |
| Écriture | Dans le clone `depots/lepton/original/master/scion/…`, puis `scion graft`. Jamais dans le trunk. |
| Git | Natif dans le clone (`git -C "$LEPTON_CLONE"`), local uniquement ; le trunk n'est pas un dépôt. |
| Build | `$LEPTON_BUILD` (`<ROOTSTOCK>/build/`), hors du trunk et hors du clone. |
| Sorties de mklepton | Conflit ouvert (les mkconf écrivent dans le trunk) : à trancher à l'étape 2. |
| Conteneur | Monter le rootstock entier : les liens sont relatifs à lui. |
| Mise à jour des clones | `scion seed-update` / `graft-update` (git pull) : décision humaine. |
| `.gitignore` du clone | Hérité de Visual Studio/IAR : ignore notamment `[Dd]ebug/`, `[B]in/`, `*.bin`, `*.a`, `[Ss]ettings/`. Vérifier `git status --ignored` avant chaque commit ajoutant des fichiers. |

---

## 5. Conventions  [HUMAIN + AGENT]

- Langue : français (commentaires migration, docs) ; ton neutre et concis.
- Abstractions : extensions compilateur via `compiler.h` (macros `__lepton_*`) uniquement.
- Commits : mécaniques (un par répertoire × règle, message citant le script) strictement
  séparés des sémantiques (petits, relus). Branche `migration/etape-N`.
- Sorties volumineuses (audits, maps, inventaires) dans `doc/migration/`, jamais en
  conversation ni recopiées ici.

---

## 6. Méthode de travail  [HUMAIN]

### 6.1 Point d'entrée
Toute session de migration commence par `doc/migration/ORCHESTRATION.md` : lire
`MIGRATION-STATUS.md`, déterminer l'étape courante, mode plan, exécuter, valider, clore.
Exception : l'étape 0 (amorçage) est lancée depuis le paquet décompressé, avant que le clone
existe ; les étapes suivantes par `scripts/claude-lepton.sh`, à la racine du clone.

### 6.2 Règle anti-invention
Toute information manquante (offset, format, convention d'empilement, version) doit être
cherchée dans le code, la doc du port livré, ou les rapports d'audit. Introuvable →
question à l'utilisateur ; hypothèse indispensable → marquer `HYPOTHÈSE À VALIDER :` dans
le code et la lister au compte rendu. Jamais de supposition silencieuse.

### 6.3 Points d'arrêt
S'arrêter et faire valider : chaque décision du tableau §4 d'`ORCHESTRATION.md` ; fin de
chaque étape même si tout est vert ; avant toute opération irréversible (écriture flash,
suppression de code gelé, retrait d'IAR) ; après trois échecs successifs sur un même point.

### 6.4 Comptes rendus
Clore chaque session par : fait / **vérifié et comment** / incertain / prochaine action,
la mise à jour de `MIGRATION-STATUS.md` et du handoff (`doc/migration/handoff/`, ORCHESTRATION §3bis). Ne pas présenter comme validé ce qui n'a été
que compilé.

---

## 7. Zones sensibles  [HUMAIN]

| Chemin / ressource | Régime |
|---|---|
| code gelé (`code-gele.md`), `third_party/` | Lecture seule |
| fichiers `.ewp`/`.eww`/`.icf`, asm IAR | Lecture seule (source d'information) jusqu'à leur suppression (étape 6) |
| `compiler.h`, CMake communs, `MIGRATION-STATUS.md` | Session principale uniquement (jamais un sous-agent) |
| `trunk/` | Lecture seule — vue composée ; toute écriture passe par le clone. Bloqué par le hook `.claude/hooks/lepton_guard.py` |
| `depots/lepton-seed.scions/`, `depots/generation/` | Lecture seule — seed et emplacement gérés par scion |
| Dépôt `seed.scions` (outil scion) | Hors périmètre — projet distinct ; défauts signalés par issue, jamais corrigés ici |
| `scion seed-update`, `graft-update`, `graft-clean` | Point d'arrêt — validation explicite requise |
| Écriture flash sur cible | Validation explicite requise |
| périmètre actif hors fichiers de l'étape en cours | Point d'arrêt avant modification |

---

## 8. Validation  [HUMAIN]

**Bancs :** QEMU `mps2-an386` (UART CMSDK, puis Ethernet LAN9118) : test de fumée
(`smoke_lsh.py` : démarrage → `lsh` → `uname -a`) et banc KAL
(`doc/migration/BANC-TEST-KAL-QEMU.md`) ; puis NUCLEO-F439ZI via sonde USB.

**Oracle :** aucun binaire IAR. Oracles : exécution (paliers des étapes 3 et 5), sorties de
référence de mklepton (étape 1), comparaison embOS / FreeRTOS test par test (étape 7).

**Critères d'acceptation permanents :**
- [ ] Socle QEMU vert à chaque commit dès l'étape 3.
- [ ] `audit_iar.py` décroissant (étapes 3-4) puis zéro sur le périmètre actif.
- [ ] Critères du fichier `ETAPE-N` courant intégralement cochés avant passage à N+1.

---

## 9. Points ouverts  [AGENT OK]

<!-- Liste vivante — reprendre aussi le tableau des décisions d'ORCHESTRATION.md §4. -->
- [ ] BSP embOS le plus proche du F439 (étape 1).
- [ ] Discovery F7 : modèle exact (étape 6).
- [ ] Noyau statique : pilote logiciel manquant (guide cité deux fois `dev_null`) ; stockage de
  l'image UFS (étape 2).
- [ ] Contenu de `bin` : pseudo-binaires de tests unitaires (étape 3).
- [ ] mklepton : sources localisables ? (étape 1 ; repli à arbitrer seulement si introuvables).
- [ ] Licence embOS : évaluation vs production (étape 1).
- [x] Trunk : chemins mkconf rendus relatifs (à faire à l'étape 2) ; rootstock `~/lepton`,
  trunk `trunk/` ; Claude Code lancé à la racine du clone par `scripts/claude-lepton.sh`
  (décision 2026-09-30).

---

## 10. Documentation de référence

| Fichier | Contenu |
|---|---|
| `doc/migration/README.md` | plan global, environnement hôte, graphe des étapes |
| `doc/migration/ORCHESTRATION.md` | machine à états, décisions humaines, discipline git, agents |
| `doc/migration/MIGRATION-STATUS.md` | avancement, matrices, versions épinglées, blocages |
| `doc/migration/ETAPE-0..7, BANC-TEST-KAL-QEMU, ANNEXE-nouveau-coeur-riscv` | instructions d'étape |
| `doc/migration/ajout-coeur.md` | procédure d'ajout d'un cœur ou d'une carte (étape 2) |
| `doc/migration/sources/lepton-migration-guide-step-1.md` | guide de l'auteur : noyau statique, mklepton, noyau dynamique |
| README de `lepton-distribution/seed.scions` (tag 0.5.0.1) | commandes, formats `.scion.*`, layout rootstock |
| `seed.scions/doc/migration/MIGRATION-STATUS.md` | document du projet scion (externe) ; constats sur l'arbre Lepton repris dans `doc/migration/MIGRATION-STATUS.md` |
