# Étape 0 — Arbre des sources Lepton avec scion 0.5

<!-- Provenance : README et CHANGELOG de lepton-distribution/seed.scions (tag 0.5.0.1 = 54dc319),
     statut de référence seed.scions/doc/migration/MIGRATION-STATUS.md (vérifié le 2026-09-29),
     seed lepton-seed.scions (branche original-tree, 083c30b). Méthode cdc-to-instructions.
     Faits recoupés par exécution réelle le 2026-09-29. Premier jet à relire.
     Numérotation du plan retenue (décision 2026-09-29) : ETAPE-0 = reconstitution de l'arbre ;
     la réécriture de scion, « étape 0 » dans le MIGRATION-STATUS de seed.scions, en est un prérequis
     terminé. -->

## Contexte

L'arbre des sources Lepton est **composé** par `scion` (dépôt `lepton-distribution/seed.scions`)
à partir de scions issus de dépôts git, décrits par un seed. Depuis 0.5.0.1 : greffe **par
feuilles** (répertoires réels, fichiers = liens symboliques **relatifs** vers `depots/`), plus de
variable `SCION_ROOTSTOCK`, plus de Windows. La réécriture de l'outil est terminée ; cette étape
reconstitue le rootstock de travail et fixe les règles d'édition des étapes suivantes.
Les fichiers du plan vivent dans le dépôt `lepton-distribution/lepton-original-tree.scions`
(décision 2026-09-29) : cette étape est donc un **amorçage** — lancée depuis le paquet de
passation décompressé, hors dépôt (`README-PAQUET.md`), elle crée le clone qui hébergera ensuite
le plan (tâche 6). Lire avant de
commencer `seed.scions/doc/migration/MIGRATION-STATUS.md` (constats vérifiés sur l'arbre).

## Prérequis

- Debian natif, `git` ≥ 2.30, Python ≥ 3.10, `pipx` (installés par `scripts/install-debian.sh`).
- Accès réseau à GitHub.
- Décision sur le nom et l'emplacement du trunk actée (tâche 2) : elle conditionne mklepton.

## Tâches

### 1. Installer scion, version épinglée

```bash
pipx install "git+https://github.com/lepton-distribution/seed.scions.git@0.5.0.1"
scion version        # attendu : scion version: 0.5.0.1
```

Le MIGRATION-STATUS du dépôt indique `git clone -b 0.5.0.1 … && pip install ./seed.scions` :
sous Debian (PEP 668), `pip install` dans le Python système est refusé — utiliser pipx, comme le
README de l'outil le recommande. Ne pas installer depuis `master` (postérieur au tag).

### 2. Choisir le nom et l'emplacement du trunk — point d'arrêt

Les configurations de build existantes codent en dur `$(HOME)/tauon/…` :
`scion/tools/bin/mklepton_gnu.sh` (substitution de `$(HOME)` par `sed`) et quatre
`scion/sys/user/tauon_sampleapp/etc/mkconf_tauon_sampleapp*.xml` (`dest_path` des sorties).
Deux issues, à trancher par l'humain avant de greffer :

1. rootstock = `$HOME`, `scion rootstock-install --trunk tauon` : reproduit l'arborescence
   attendue, débloque immédiatement, mais fige nom et emplacement (gênant en CI/conteneur) ;
2. rendre les chemins des mkconf relatifs au trunk (traité à l'étape 2) : libère nom et
   emplacement. Solution cible à terme selon le MIGRATION-STATUS du dépôt.

### 3. Créer le rootstock et greffer

```bash
cd <ROOTSTOCK>                                # selon la décision de la tâche 2
scion rootstock-install [--trunk tauon]
scion seed-add --version original-tree \
      https://github.com/lepton-distribution/lepton-seed.scions.git
scion graft
```

```
<ROOTSTOCK>/
├── .scion.rootstock.signature
├── depots/
│   ├── lepton/original/master/            clone de lepton-original-tree.scions (git)
│   ├── lepton-seed.scions/original-tree/  clone du seed
│   ├── generation/building/               emplacement de génération
│   └── origin/scion/sources/
└── <trunk>/                               arbre composé : sys/ tools/ building/
```

### 4. Vérifier (valeurs de référence du 2026-09-29)

`source scripts/lepton-env.sh <ROOTSTOCK>` (script du paquet) exporte `LEPTON_ROOTSTOCK`,
`LEPTON_TRUNK`, `LEPTON_CLONE`, `LEPTON_BUILD`, utilisés par toutes les étapes.

- 4829 feuilles greffées ; `scion rootstock-information` → 2 scions.
- `<trunk>/sys/root/src/kernel/core/kal.h` et `<trunk>/tools/bin/mklepton_gnu` présents.
- `git -C "$LEPTON_CLONE" status --porcelain` vide.
- `find "$LEPTON_TRUNK" -xtype l` vide (aucun lien cassé), y compris après déplacement du rootstock.
- `find "$LEPTON_TRUNK" -type f ! -name .scion.grafted.list` vide (aucun fichier régulier).

### 5. Règles de travail (à reporter dans CLAUDE.md et ORCHESTRATION)

| Sujet | Règle |
|---|---|
| Lecture, build | Par le trunk (aucun chemin `depots/` n'est codé en dur dans l'arbre Lepton — vérifié). |
| Écriture | **Dans le clone** `depots/lepton/original/master/scion/…`, puis `scion graft`. Jamais de création ni d'édition dans le trunk. |
| Git | Commandes natives dans le clone (`git -C "$LEPTON_CLONE"`, ou `git` directement depuis la racine du clone) : branches, commits, tags, en local uniquement (push seulement avec accord explicite). Le trunk n'est pas un dépôt. |
| Build | `$LEPTON_BUILD` (`<ROOTSTOCK>/build/`), hors du trunk et hors du clone. |
| Sorties de mklepton | **Conflit ouvert** : les mkconf écrivent dans le trunk (`sys/root/src/kernel/core/arch/<cpu>`, `sys/user/tauon_sampleapp/etc`) — soit à travers un lien (modifie un fichier versionné du clone), soit en créant un fichier régulier (bloque `scion graft`). À trancher à l'étape 2. |
| Conteneur | Monter le rootstock entier : les liens sont relatifs à lui. |
| Mise à jour des clones | `scion seed-update` / `graft-update` font `git pull` : interdits sans décision humaine (échec sur une branche sans amont, code 2). |
| Chemins du plan | `src/…` est relatif à `sys/root/`. |

### 6. Installer le plan dans le dépôt Lepton

Dans le clone `depots/lepton/original/master` (dépôt `lepton-original-tree.scions`, branche
`master` — la branche `main` ne contient qu'un commit initial vide) :

```
<racine du dépôt>/                 non greffé (hors scion/)
├── CLAUDE.md                      chargé par Claude Code lancé à la racine du clone
├── doc/migration/                 README, ORCHESTRATION, ETAPE-0..7, BANC-TEST-KAL-QEMU, ANNEXE, sources/,
│   ├── MIGRATION-STATUS.md          MIGRATION-STATUS, handoff/
│   └── handoff/
├── scripts/                       install-debian.sh, lepton-env.sh, claude-lepton.sh
├── .claude/settings.json          hook PreToolUse : refuse toute écriture dans le trunk
├── .claude/hooks/lepton_guard.py
├── .claude/skills/lepton-portage-instructions/
├── tools/migration/, ci/          créés par les étapes suivantes (outillage, non greffé)
└── scion/                         greffé : sys/, tools/ … et, créés par les étapes suivantes,
                                   CMakeLists.txt, CMakePresets.json, cmake/, ld/, tests/
```

- Copier le contenu du paquet à la racine du clone. `doc/migration/MIGRATION-STATUS.md` est
  fourni prérempli (décisions actées, versions vérifiées, constats) : le compléter avec la
  décision de la tâche 2 et la révision du clone. Il devient la seule source de vérité du
  chantier Lepton.
  `seed.scions` est un **projet distinct** (outil utilisé par Lepton) : son dépôt n'est pas
  modifié par la migration ; ses défauts constatés lui sont signalés
  par une issue, jamais corrigés depuis ce chantier.
- Branche du premier commit : `migration/etape-0` (décision 2026-09-29), commit **local** ;
  aucun push sans accord explicite de l'utilisateur (décision 2026-09-30).
- À partir de l'étape 1, Claude Code est lancé par `scripts/claude-lepton.sh` : racine du clone,
  accès au trunk et au build (`--add-dir`), hook de protection actif.

### 7. Produire le handoff

`doc/migration/handoff/etape-0.md` (ORCHESTRATION §3bis) : rootstock et trunk retenus, version de
scion, révision du clone (`git -C "$LEPTON_CLONE" rev-parse HEAD`), règles actées.

## Critères de validation

- [ ] `scion version` → `0.5.0.1`, installée depuis le tag.
- [ ] Décision de la tâche 2 consignée dans `MIGRATION-STATUS.md`.
- [ ] Vérifications de la tâche 4 toutes conformes.
- [ ] `scion graft` rejoué deux fois sans erreur (idempotence).
- [ ] Règles de la tâche 5 reportées dans `CLAUDE.md`.
- [ ] Plan installé à la racine du clone (hors `scion/`), commité localement (pas de push) ; `find "$LEPTON_TRUNK" -name CLAUDE.md` vide (le plan n'est pas greffé).
- [ ] `scripts/claude-lepton.sh` démarre à la racine du clone ; une tentative d'écriture dans le
      trunk est refusée par le hook.

## Pièges connus

- **`sed -i` remplace le lien par un fichier régulier** (vérifié) : modification hors git, puis
  `scion graft` bloqué. Travailler dans le clone ; à défaut `sed -i --follow-symlinks`. Vérifier
  `spatch --in-place` avant les transformations de l'étape 3.
- `mklepton_gnu` est un exécutable ELF **i386** précompilé, lancé par `./mklepton_gnu` depuis
  `tools/bin` : il exige les bibliothèques 32 bits (`libc6:i386`) et le bon répertoire courant.
  C'est l'existant à remplacer par un mklepton natif (étape 2).
- Ne pas utiliser `scion git` : toute opération git se fait en commande native
  `git -C "$LEPTON_CLONE" …` (lecture comme écriture).
- Le clone vit dans un répertoire nommé `master` même quand une autre branche y est extraite.
- La branche `master` du seed (20 scions) n'est pas utilisée : son scion `building` pointe sur
  `depots/lepton/building`, que `rootstock-install` ne crée pas.

## À la fin de l'étape

Mettre à jour `MIGRATION-STATUS.md` (tableau des étapes : reconstitution terminée ; décision
trunk ; révision du clone) et produire le handoff.
