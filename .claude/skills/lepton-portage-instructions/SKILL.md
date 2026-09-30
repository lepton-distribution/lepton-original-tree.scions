---
name: lepton-portage-instructions
description: Transforme un besoin de portage, migration ou chantier Lepton RTOS (exprimé en texte libre, cahier des charges ou discussion de faisabilité) en fichiers d'instructions autonomes ETAPE-N pour Claude Code, cohérents avec ORCHESTRATION.md et MIGRATION-STATUS.md. Utiliser ce skill dès que l'utilisateur demande de créer, découper ou réviser un plan de travail Lepton en étapes exécutables — "prépare un plan pour X", "crée les instructions pour Y", "ajoute une étape pour Z", "découpe ce chantier", même sans prononcer "fichier d'instructions".
---

# Besoin Lepton → fichiers d'instructions ETAPE-N

Convertit un énoncé de besoin en fichiers d'instructions exploitables par Claude Code, suivant le
gabarit du projet de migration Lepton.

**Ce que ce skill produit :** un ou plusieurs fichiers `ETAPE-N-*.md`, les mises à jour associées
(`README.md` : tableau et graphe ; `ORCHESTRATION.md` : points d'arrêt), et un rapport des zones
d'incertitude.

**Ce que ce skill ne fait PAS :** explorer le code, spécifier la solution technique, trancher des
arbitrages. Le fichier produit *organise et cadre* le travail ; il ne le résout pas.

---

## Principe directeur

> Un énoncé de besoin contient trois types d'éléments : des **faits**, des **contraintes**, et des
> **manques**. La valeur des fichiers produits tient à ce qu'ils ne confondent jamais les trois.

Corollaire : **tout manque devient une question explicite** (point d'arrêt humain dans
`ORCHESTRATION.md` §4, ou `<À CONFIRMER>` dans le fichier d'étape), **jamais une hypothèse
implicite**. Une supposition écrite dans un fichier d'instructions acquiert le statut de consigne
et sera rejouée à chaque session.

---

## Procédure

### Étape 1 — Analyse de faisabilité d'abord

Lire l'énoncé en entier (les contraintes fortes sont souvent dispersées). Produire une analyse
courte — faisabilité, découpage proposé, dépendances, risques, décisions à trancher — et la faire
**valider par l'utilisateur avant de générer les fichiers**. C'est sa méthode de travail établie.

Ne pas explorer le dépôt à ce stade : l'exploration est une consigne à écrire *dans* les fichiers
produits (tâches d'audit), pas une action à mener ici.

### Étape 2 — Identifier l'archétype

Lire `references/archetypes-portage.md`. Trois cas :

| Archétype | Signal | Ce que le fichier doit accentuer |
|---|---|---|
| **Migration** (iso-fonctionnel) | « migrer », « porter de X vers Y », existant qui fait foi | périmètre, non-régression, **oracle explicite**, transformation outillée |
| **Portage nouveau** (création) | « ajouter le support de », architecture/backend inexistant | jalons à critère de sortie, parité avec la référence existante, choix préalables |
| **Infrastructure** | CI, banc de test, outillage, documentation | reproductibilité, versions épinglées, intégration à l'existant |

Se tromper d'archétype produit un fichier inutile : une migration cadrée comme une création
autorise la refactorisation d'opportunité — le pire risque sur 1 MLOC.

### Étape 3 — Classer chaque énoncé dans le gabarit

| Contenu de la source | Section du gabarit ETAPE-N |
|---|---|
| Finalité, place dans le plan, principe directeur | Contexte |
| Étapes préalables, outils, paquets, décisions déjà actées | Prérequis |
| Actions, scripts à créer, tables de traduction | Tâches |
| « ne pas toucher », code gelé, fichiers de référence | Tâches (périmètre) + Pièges |
| Résultats attendus, procédure de vérification | Critères de validation |
| Risques, symptômes connus, cas limites | Pièges connus |
| **Tout manque, ambiguïté ou contradiction** | **Point d'arrêt humain (ORCHESTRATION §4) ou `<À CONFIRMER>`** |

Un énoncé qui ne rentre nulle part est du contexte narratif : soit il éclaire une décision
(Contexte), soit il est décoratif (supprimer).

### Étape 4 — Détecter les tensions

Chercher activement : contradictions (périmètre annoncé techniquement intenable), non-dits
structurants (voir la liste de contrôle de `references/archetypes-portage.md` §5), exceptions
énoncées en passant (« sauf… » en milieu de phrase — les remonter en tête de section), conventions
révélatrices. Ces tensions ne se tranchent pas : elles vont au rapport final et au tableau des
décisions, mention « à arbitrer ».

### Étape 5 — Rédiger

Gabarit obligatoire (celui des fichiers ETAPE-0 à 7 existants) :

```markdown
# Étape N — <titre>

## Contexte
<2-5 phrases : rappel Lepton pertinent, place dans le plan, principe directeur.
Autonome : lisible sans les autres fichiers ni la conversation d'origine.>

## Prérequis
<étapes préalables, outils, décisions actées>

## Tâches
### 1. <tâche>
<concret : chemins réels, noms de fichiers/scripts, commandes, tableaux de
correspondance quand il y a traduction ; jamais de généralités>

## Critères de validation
- [ ] <vérifiable mécaniquement quand possible (script, build, diff, compteur)>

## Pièges connus
<spécifiques et actionnables (symptôme → cause → parade)>

## À la fin de l'étape
Mettre à jour `doc/migration/MIGRATION-STATUS.md` : <quoi exactement>.
```

Règles de rédaction :

- **Chaque ligne doit changer une décision de l'agent.** Test de relecture : « si je supprime
  cette ligne, l'agent agira-t-il différemment ? » Si non, supprimer.
- **Impératif, pas descriptif.** « Relancer audit_iar.py après chaque lot », pas « il serait
  souhaitable de vérifier ».
- **Critères vérifiables.** « `mass_compile.sh` à 100 % sur le périmètre actif », pas « le code
  compile bien ».
- **Traçabilité.** Toute contrainte provient d'une phrase de la source ou des conventions du
  projet (§ ci-dessous) ; tout ajout de bonne pratique générale est signalé dans le rapport.
- **`<À CONFIRMER>`** pour tout champ sans réponse dans la source. Jamais de déduction plausible.
- **Volumineux → fichiers.** Inventaires, spécifications, journaux vont dans `doc/migration/`,
  référencés, jamais recopiés dans le fichier d'étape.
- **Une variable par étape ; un état compilable/validable par étape** ; chantiers indépendants =
  étapes séparées d'ordre interchangeable.

### Étape 6 — Conventions du projet Lepton (à respecter dans tout fichier)

- Langue : français ; ton neutre, objectif, concis.
- Environnement : Debian natif (`scripts/install-debian.sh`), Claude Code lancé par
  `scripts/claude-lepton.sh` à la racine du clone ; variables `LEPTON_*` (`scripts/lepton-env.sh`) ;
  build dans `$LEPTON_BUILD` ; flash par OpenOCD sur la même machine ; versions épinglées dans
  `MIGRATION-STATUS.md`.
- Arbre des sources : rootstock `scion` 0.5.0.1 (ETAPE-0) — toute écriture et tout git dans le
  clone `depots/lepton/original/master`, jamais dans `trunk/` ; build depuis `trunk/`, hors trunk ;
  chemins `src/…` relatifs à `sys/root/`.
- Bibliothèques statiques par composant (`lepton_core`, `lepton_vfs`, `lepton_fs_<fs>`,
  `lepton_dev`, `lepton_bsp_<carte>`, `lepton_libc`, `lepton_sbin`, `lepton_bin`…) ; `bin`, `sbin`,
  `lib` hors de `kernel/`. Non-régression : `ci/run.sh`.
- Volumétrie ~1 MLOC : tâches de masse outillées (scripts rejouables), pilotées par métriques,
  bornées au périmètre actif (matrice fichier × cible).
- Cibles : QEMU `mps2-an386` (socle), NUCLEO-F439ZI (base), puis M7, M3, M0/M0+. Gelés : ARM7,
  ARM9, M16C, simulations. RISC-V reporté (annexe) ; architecture à quatre axes ISA / cœur /
  carte / micro-noyau (`ajout-coeur.md`).
- Micro-noyau : backend embOS (port GCC Segger) ; FreeRTOS = étape 7 ; KAL décomposé
  `kal/arch/<famille>/` × `kal/backend/<rtos>/`.
- Validation par exécution : socle QEMU (test de fumée, banc KAL), puis carte. IAR n'est pas une
  référence de comparaison (abandonné).
- Réutiliser les scripts existants (`audit_iar.py`, `mass_compile.sh`,
  `build_closure.py`, banc KAL) — ne jamais dupliquer un mécanisme sous un autre nom.
- Toute décision humaine nouvelle est ajoutée au tableau §4 d'`ORCHESTRATION.md`.

### Étape 7 — Contrôle qualité avant livraison

- [ ] Chaque contrainte rattachable à la source ou aux conventions ; ajouts signalés.
- [ ] Aucun chemin, version, constante ou valeur inventé ; recopie caractère pour caractère.
- [ ] Les exceptions de la source apparaissent (premier oubli classique).
- [ ] Au moins une question ouverte ou décision à arbitrer — un énoncé sans zone d'ombre
      n'existe pas ; zéro question = des trous comblés silencieusement.
- [ ] Archétype cohérent (périmètre/non-régression durcis si migration ; jalons si création).
- [ ] Au moins un point d'arrêt avant la première modification de code.
- [ ] Fichier d'étape autonome et < 200 lignes.
- [ ] Numérotation, tableau README, graphe de dépendances et ORCHESTRATION §4 mis à jour.

### Étape 8 — Livrer

Produire les fichiers, puis un rapport court : emplacement (`doc/migration/`), liste des
`<À CONFIRMER>` et décisions à arbitrer, tensions détectées, ajouts hors source. Présenter comme
un premier jet à relire, pas comme définitif.

---

## Ressources

- `references/archetypes-portage.md` — les trois archétypes appliqués à Lepton, formulations
  efficaces vs inefficaces, liste de contrôle des non-dits structurants d'un portage embarqué.
