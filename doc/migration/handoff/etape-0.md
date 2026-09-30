# Handoff étape 0 → 1

## Réponses aux prérequis de 1
- Rootstock scion : `/home/lepton-user/lepton` (`~/lepton`), signature `.scion.rootstock.signature`.
- Clone : `~/lepton/depots/lepton/original/master` (`lepton-original-tree.scions`), branche
  `migration/etape-0` créée sur `master` `055fc602f32f81a7303aad0a99eb8f821c85ad5a`.
- Trunk greffé : `~/lepton/trunk` (nom par défaut), 4829 liens relatifs, 2 scions, seed
  `original-tree` `083c30b`.
- Chemins : `source scripts/lepton-env.sh` exporte `LEPTON_ROOTSTOCK`, `LEPTON_TRUNK`,
  `LEPTON_CLONE`, `LEPTON_BUILD` (`~/lepton/build`, créé par `claude-lepton.sh`).
- Audits : le trunk ne contient que des liens → `find -L "$LEPTON_TRUNK"` ou lecture directe
  de `$LEPTON_CLONE/scion/…` ; chemins de rapport relatifs à `sys/root/` ou `tools/`.

## Décisions actées pendant 0
- 2026-09-30 : chemins des mkconf rendus relatifs au trunk (travail de l'étape 2) ; pas de
  `tauon` dans `$HOME`. Rootstock `~/lepton`. Lancement par `scripts/claude-lepton.sh`.
- 2026-09-30 : scion 0.5.0.1 conservé, installé par pipx depuis un clone local de
  `seed.scions` (`e0adb2c`, code identique au tag `54dc319`) — écart consigné.
- 2026-09-30 : `archives/embOS` du paquet (Segger, sous licence) non versionné ; il reste
  dans `~/Development/lepton-migration-development/archives/embOS` (doc UM01001, UM01039,
  `Start/`) — utile pour le BSP F439 et la licence (étape 1).

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu et quand le lire |
|---|---|
| `CLAUDE.md` §4 | règles de l'arbre composé (tâche 5) ; toujours chargé |
| `doc/migration/MIGRATION-STATUS.md` | décisions datées, versions, constats ; début de session |
| `scripts/*.sh`, `.claude/` | outillage du paquet, rendus exécutables |

## Vérifications faites (2026-09-30)
- `scion version` 0.5.0.1 ; `scion rootstock-information` → 2 scions.
- `kal.h` et `tools/bin/mklepton_gnu` présents ; `find -xtype l` et `find -type f` vides ;
  clone propre après greffe ; déplacement aller-retour du rootstock sans lien cassé ;
  `scion graft` rejoué deux fois (rc 0).
- `find "$LEPTON_TRUNK" -name CLAUDE.md` vide.
- Hook : Edit/Write/`sed -i`/redirection vers le trunk refusés (rc 2), push → demande d'accord ;
  `claude-lepton.sh -p` démarre à la racine du clone et une écriture Write dans le trunk est
  refusée. Non testé : lancement interactif (dialogue de confiance au premier lancement).

## Écarts au plan et pièges découverts
- `.gitignore` du clone hérité de Visual Studio/IAR : ignore `[Dd]ebug/` (donc le `debug/`
  prévu à l'étape 5), `[B]in/`, `*.bin`, `*.a`, `*.i*86`, `[Ss]ettings/`, `bld/`, `*.log`.
  Contrôler `git status --ignored` avant chaque commit d'ajout ; modifier le `.gitignore`
  est une décision sémantique à proposer (étape 2 au plus tard).
- `scripts/claude-lepton.sh` n'avait pas le bit exécutable dans le paquet (corrigé au commit).
- Aucun écart sur les valeurs de référence (4829 feuilles, `055fc60`, `083c30b`).

## Non transmis volontairement
- Sortie détaillée de `scion seed-add` / `graft` : rejouable, sans information utile.
- Paquet de passation d'origine : `~/Development/lepton-migration-development` (copie
  intégrale versionnée ici, sauf `archives/`).
