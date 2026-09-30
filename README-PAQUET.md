# Paquet de passation — migration Lepton IAR/Windows → GCC/Linux

Ce paquet contient tout ce dont Claude Code a besoin pour conduire la migration. Son arborescence
est celle qu'il aura **à la racine du dépôt `lepton-original-tree.scions`** (hors de `scion/`,
donc non greffée dans l'arbre de build).

```
CLAUDE.md                              chargé automatiquement par Claude Code
doc/migration/
  README.md                            plan global, principes, table des étapes
  ORCHESTRATION.md                     machine à états, points d'arrêt, git, agents, handoff
  MIGRATION-STATUS.md                  état prérempli : décisions, versions, constats
  ETAPE-0-arbre-sources.md … ETAPE-7-backend-freertos.md
  BANC-TEST-KAL-QEMU.md                banc transversal du KAL
  ANNEXE-nouveau-coeur-riscv.md        RISC-V reporté, exigences d'architecture
  sources/lepton-migration-guide-step-1.md   guide de l'auteur
  handoff/                             passages de contexte entre sessions (vide)
scripts/
  install-debian.sh                    prérequis (Debian natif) ; --with-debug-tools, --with-riscv
  lepton-env.sh                        à sourcer : LEPTON_ROOTSTOCK, _TRUNK, _CLONE, _BUILD
  claude-lepton.sh                     lance Claude Code à la racine du clone (+ trunk, build)
.claude/
  settings.json                        hook PreToolUse
  hooks/lepton_guard.py                refuse l'écriture dans le trunk ; exige votre accord pour tout push
  skills/lepton-portage-instructions/  skill de rédaction de nouvelles étapes
```

## Démarrage

1. **Prérequis** (une fois, sur le PC Debian) :

   ```bash
   scripts/install-debian.sh --with-debug-tools
   ```

   Vérifier : `scion version` → 0.5.0.1 ; `arm-none-eabi-gcc --version` ; `qemu-system-arm --version`.

2. **Étape 0 — amorçage**, depuis le paquet décompressé (le clone n'existe pas encore) :

   ```bash
   cd lepton-migration-package
   claude "Lis doc/migration/ORCHESTRATION.md puis exécute doc/migration/ETAPE-0-arbre-sources.md." \
       --add-dir <répertoire qui contiendra le rootstock>
   ```

   La consigne doit précéder `--add-dir` : cette option accepte plusieurs répertoires et prendrait
   sinon la consigne pour l'un d'eux (Claude Code s'ouvrirait alors sans rien faire). Au premier
   lancement, accepter la confiance dans le dossier (active le hook de `.claude/settings.json`).

   Claude Code s'arrêtera sur la décision du trunk (`tauon` dans `$HOME`, ou chemins mkconf
   relatifs), créera le rootstock, greffera l'arbre, copiera ce paquet à la racine du clone et
   committera localement sur la branche `migration/etape-0`, sans pousser.

3. **Étapes suivantes**, depuis le clone :

   ```bash
   <ROOTSTOCK>/depots/lepton/original/master/scripts/claude-lepton.sh \
       "Lis doc/migration/ORCHESTRATION.md et poursuis la migration."
   ```

   Chaque session : mode plan soumis à validation, exécution, critères de validation, handoff,
   `MIGRATION-STATUS.md`, commit local (aucun push sans votre accord) ; arrêt en fin d'étape.

## Décisions qui vous reviennent en cours de route

Liste tenue dans `MIGRATION-STATUS.md` (section « Décisions ouvertes ») et ORCHESTRATION §4.
Les plus proches : nom du trunk (étape 0), BSP embOS du F439 et licence (étape 1), second pilote
logiciel du noyau statique, stockage de l'image UFS, sorties de mklepton (étape 2).

## Garde-fous

- **Git local uniquement** : aucun push sans votre accord. Le hook `.claude/hooks/lepton_guard.py`
  transforme toute commande `git push` (ou `gh pr create`) en demande d'accord, à chaque fois.
- `CLAUDE.md` est du contexte, pas une contrainte. Les blocages réels sont ceux du hook : push
  (demande d'accord) et écriture dans le trunk (refus : outils d'édition, `sed -i`,
  `spatch --in-place`, redirections ; inactif tant qu'aucun rootstock n'existe).
- Le code gelé et `third_party/` ne sont protégés que par les consignes : pour un blocage réel,
  étendre le hook ou retirer les droits d'écriture.
