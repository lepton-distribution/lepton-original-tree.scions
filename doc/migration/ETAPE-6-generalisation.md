# Étape 6 — Généralisation aux autres cœurs et cartes, CI, retrait d'IAR

## Contexte

La base de départ est posée (NUCLEO-F439ZI, étape 5). Cette étape étend Lepton aux autres cœurs
et cartes **en appliquant `doc/migration/ajout-coeur.md`** (étape 2) : chaque ajout ne crée que des
fichiers aux emplacements prévus. Elle met en place la CI et retire définitivement IAR du dépôt.
L'état final est mono-chaîne GCC sous Linux.

## Prérequis

- Étape 5 close.
- Liste des cartes à traiter actée par l'utilisateur (point d'arrêt en début d'étape).

## Tâches

### 1. Nouveaux cœurs et cartes

Ordre : chaque cœur d'abord sous QEMU (quand une machine existe), puis sur carte.

| Cœur | QEMU | Carte | Remarque |
|---|---|---|---|
| M4F (2ᵉ carte) | — | Olimex STM32-P407 | disponible ; BSP existant dans l'arbre (`prj/iar/bsp/olimex_p407`) ; Ethernet |
| M7 | `mps2-an500` (LAN9118 à `0xA0000000`) | Discovery F7, modèle <À CONFIRMER> | disponible ; cache et FPU double précision |
| M3 | `mps2-an385` | à choisir | BSP existants dans l'arbre : `stm32f1xx`, `lm3s` |
| M0/M0+ | `microbit` (UART seule) | à choisir | BSP existant : `samd20xplained_pro` (SAMD20) ; famille `armv6m` |

- Pour chaque ajout : suivre `ajout-coeur.md` ; rejouer `transform_iar.py` et `mass_compile.sh` sur
  le seul code propre à la carte (périmètre différé) ; paliers de l'étape 3 sous QEMU, puis de
  l'étape 5 sur carte ; statut « validé sur carte » ou « QEMU seulement » dans `MIGRATION-STATUS.md`.
- Toute modification du code commun demandée par un ajout est un défaut de l'architecture de
  l'étape 2 : la corriger et mettre à jour `ajout-coeur.md`.

### 2. CI

- `ci/Dockerfile` (Debian épinglée, construit sur `scripts/install-debian.sh`) ; `ci/run.sh`
  (existant depuis l'étape 3) étendu à tous les presets ; le rootstock scion est monté entier.
- Pipeline : build de tous les presets ; `ctest -L smoke` et `ctest -L kal` sur toutes les machines
  QEMU ; `audit_iar.py` en garde (zéro IAR-isme hors code gelé) ; occupation mémoire archivée
  (`--print-memory-usage`) avec seuil d'alerte ; artefacts `.elf`/`.bin`/`.map`.
- Optionnel : exécution sur la F439 par un exécutant équipé de la sonde.

### 3. Retrait d'IAR

- Supprimer du dépôt les `.ewp`, `.eww`, `.ewd`, `.icf` et l'assembleur en syntaxe IAR, après un
  tag `legacy-iar` sur l'état précédent.
- `compiler.h` : aucune branche IAR (déjà le cas depuis l'étape 3).

### 4. Code gelé et documentation

- Sort du code gelé (`code-gele.md`) : suppression après tag `legacy`, ou conservation documentée —
  décision de l'utilisateur, jamais de statu quo implicite.
- Documentation Doxygen : procédure de build, table cœurs × cartes × statut ; `doc/BUILDING.md` à jour.

## Critères de validation

- [ ] Chaque cœur retenu : preset, QEMU vert (si machine), carte validée ou statut explicite.
- [ ] Aucun ajout n'a modifié le code commun hors de la ligne d'enregistrement prévue.
- [ ] CI verte et reproductible localement.
- [ ] Plus aucun fichier IAR dans la branche principale ; tag `legacy-iar` posé.
- [ ] Décision sur le code gelé appliquée.

## Pièges connus

- Ne pas dupliquer le démarrage par MCU : un démarrage par famille, une table de vecteurs par MCU.
- `mps2-an500` : adresse de l'Ethernet différente des autres MPS2 — elle vient de `cmake/boards/`.
- Épingler la version de `gcc-arm-none-eabi` validée à l'étape 5 ; tout changement de version
  rejoue les paliers.

## À la fin de l'étape

`MIGRATION-STATUS.md` final de la migration : table cœurs × cartes × statut, dette restante ;
`handoff/etape-6.md`.
