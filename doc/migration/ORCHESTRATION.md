# ORCHESTRATION — Conduite de la migration Lepton par Claude Code

Ce fichier est le point d'entrée unique. Il définit comment enchaîner les fichiers `ETAPE-N-*.md`, gérer l'état entre sessions, les points d'arrêt humains, le mode plan et l'usage d'agents. Lancement type :

```bash
scripts/claude-lepton.sh "Lis doc/migration/ORCHESTRATION.md et poursuis la migration."
# étape 0 (amorçage) : voir README-PAQUET.md
```

## 1. Machine à états

À chaque session, dans l'ordre :

1. **Lire `doc/migration/MIGRATION-STATUS.md`** (fourni prérempli par le paquet ; §6). Il est l'unique source de vérité de l'avancement — ne jamais déduire l'état du dépôt par exploration quand le statut le documente.
2. **Déterminer l'action courante** : première étape dont le statut n'est pas `TERMINÉ`, ou le module suivant dans l'étape en cours (étape 3 : sessions 3a/3b ; étapes 4 et 6 : par module/carte). Charger ensuite le handoff correspondant (`doc/migration/handoff/`, §3bis) — et rien d'autre par défaut.
3. **Vérifier les prérequis** de l'étape (section « Prérequis » du fichier d'étape). Prérequis manquant → s'arrêter et le signaler, ne pas contourner.
4. **Vérifier les décisions humaines requises** (§4). Décision non actée dans le statut → poser la question à l'utilisateur et s'arrêter là.
5. **Mode plan** : présenter le plan de la session (voir §2) et attendre l'approbation.
6. **Exécuter**, en respectant la discipline git (§5) et les bornes de session (§3).
7. **Valider** : dérouler les critères de validation du fichier d'étape. Un critère en échec → l'étape reste `EN COURS`, consigner le blocage, ne jamais passer outre.
8. **Clore la session** : produire ou compléter le handoff (§3bis), mettre à jour `MIGRATION-STATUS.md` (statut, métriques, décisions, blocages), commit **local** (aucun push, §5), et produire un résumé court à l'utilisateur : fait / reste / blocages / prochaine action.

Ordre des étapes : 0 → 1 → 2 → 3 → 4 → 5 → 6 → 7, strictement linéaire. Le banc `BANC-TEST-KAL-QEMU.md` est transversal : construit à l'étape 3, rejoué à chaque lot de l'étape 4, exigé aux étapes 6 et 7. Le RISC-V (`ANNEXE-nouveau-coeur-riscv.md`) est hors séquence : il n'est pas exécuté, seules ses exigences d'architecture sont vérifiées aux étapes 2, 4 et 6.

## 2. Mode plan — systématique en début d'étape

Chaque étape (et chaque module des étapes 4 et 6) commence en **mode plan** : lecture du fichier d'étape et des fichiers concernés, puis proposition d'un plan d'exécution concret (fichiers touchés, scripts à écrire, ordre, risques) soumis à l'utilisateur avant toute modification. Justification : les fichiers d'étape disent *quoi* faire ; le plan de session dit *comment* sur le code réel, et c'est là que les mauvaises surprises (arborescence différente de l'attendu, volumétrie) doivent être détectées — avant l'exécution, pas pendant.

Exceptions : les sessions purement lecture (étape 1, audits) peuvent exécuter directement leurs scripts d'inventaire ; les reprises de session au milieu d'un module déjà planifié reprennent le plan approuvé.

## 3. Bornes de session et gestion du contexte

- **Une session = une étape**, ou un module d'étape (étape 4 : un répertoire ; étape 6 : une carte). Jamais deux étapes dans une session.
- Ne jamais charger l'arbre source entier en contexte. Travailler par scripts (grep, audit, transformation) qui produisent des synthèses, et ne lire intégralement que les fichiers en cours de modification manuelle. Si le graphe Graphify est disponible (généré à l'étape 1, exposé en MCP), l'interroger d'abord pour l'exploration structurelle (voisinage, chemins, communautés) — en gardant à l'esprit ses limites : relations macro-générées incomplètes, aide à l'exploration et non source d'autorité. Le graphe est **régénéré en fin d'étape** (et en fin de chaque module de l'étape 4) : après une transformation de masse, un graphe non régénéré est faux.
- Les sorties volumineuses (audits, listes, maps) vont dans des fichiers sous `doc/migration/`, pas dans la conversation.
- En fin de session, `MIGRATION-STATUS.md` doit permettre à une session vierge de reprendre sans relire la conversation : c'est le critère de qualité de la mise à jour.

## 3bis. Passage de contexte entre tâches (handoff)

Chaque session repart de zéro : le contexte transmis à la tâche suivante est **sélectionné, jamais accumulé**. Mécanisme :

- **Le contrat est défini par le récepteur** : la section « Prérequis » du fichier `ETAPE-N+1` (écrite à l'avance) définit ce que la fin de l'étape N doit fournir. Le handoff y répond point par point.
- **Fin d'étape N** (et fin de chaque module des étapes 4 et 6) : produire `doc/migration/handoff/etape-N.md` (ou `etape-4-<module>.md`), **≤ 100 lignes**, selon le gabarit :

```markdown
# Handoff étape N → N+1
## Réponses aux prérequis de N+1
<point par point, une ligne chacun>
## Décisions actées pendant N
## Artefacts produits (manifeste, pas copie)
| Fichier | Une ligne : ce qu'il contient et quand le lire |
## Écarts au plan et pièges découverts
<ce qui différait de ce que le fichier ETAPE-N supposait>
## Non transmis volontairement
<ce qui a été jugé inutile pour N+1, et où le retrouver si besoin>
```

- **Début de session N+1** : lecture dans cet ordre — `ORCHESTRATION.md`, `MIGRATION-STATUS.md`, `ETAPE-N+1`, `handoff/etape-N.md`. Rien d'autre par défaut : les artefacts listés dans le manifeste ne sont ouverts que si la tâche en cours l'exige.
- **Récupération** : un handoff est une compression avec perte assumée. Si une information manque, N+1 la retrouve dans les artefacts complets, l'historique git ou le graphe Graphify — puis **complète le handoff** pour la session suivante, plutôt que de reconstituer silencieusement.
- Répartition des rôles : `MIGRATION-STATUS.md` = état global grossier (où en est-on) ; `handoff/` = transition précise (de quoi la tâche suivante a besoin). Ne pas dupliquer l'un dans l'autre.
- La section « À la fin de l'étape » de chaque fichier `ETAPE-N` inclut désormais implicitement la production du handoff.

## 4. Points d'arrêt humains (ne jamais décider à la place de l'utilisateur)

Décisions à faire acter explicitement, consignées avec date dans `MIGRATION-STATUS.md` :

| Étape | Décision |
|---|---|
| 0 | Trunk `tauon` dans `$HOME` (compatible mkconf) ou mkconf relatifs ; emplacement du rootstock ; répertoire de lancement de Claude Code ; emplacement des fichiers de migration (racine du clone ou sous `scion/`) |
| 0+ | Toute mise à jour des clones (`scion seed-update` / `graft-update`) pendant la migration |
| 1 | Carte de base : NUCLEO-F439ZI (actée), précédée du socle QEMU `mps2-an386` ; vérifier le BSP embOS le plus proche |
| 1 | Sort du code gelé (`code-gele.md`, simulations incluses) — proposition oui, décision non |
| 1 | mklepton : repli (pré-générés) **uniquement si** les sources sont inutilisables — le portage natif est la voie nominale (étape 2) |
| 1 | Licence embOS : usage évaluation vs production |
| 2 | Emplacement des sorties de mklepton (hors du trunk) ; stockage de l'image UFS dans le noyau statique ; format UFS non identique hôte / ARM (types ou 32 bits) ; pilote logiciel manquant (`dev_null` cité deux fois dans le guide) |
| 3 | Contenu de `bin` (pseudo-binaires de tests unitaires) |
| 3 | Frontière libc newlib / API POSIX Lepton |
| 5 | Passage de `-Og` à `-Os`/`-O2` après validation |
| 6 | Liste des cartes (Discovery F7 : modèle ; cartes M3 et M0+) ; suppression des fichiers IAR ; sort du code gelé |
| 7 | Devenir du backend embOS après validation FreeRTOS |
| — | Lancement du portage RISC-V (annexe) |

En outre : fin d'étape = arrêt systématique et validation utilisateur avant d'entamer la suivante, même si tout est vert.

## 5. Discipline git

- Git se pratique **en commande native dans le clone** (répertoire de lancement, `$LEPTON_CLONE`), jamais `scion git`, jamais dans le trunk (vue de liens, pas un dépôt). Tout fichier nouveau est créé dans le clone puis rendu visible par `scion graft` ; le hook `.claude/hooks/lepton_guard.py` refuse toute écriture dans le trunk.
- Une branche par étape : `migration/etape-N` (ou `migration/etape-4-<repertoire>` pour les modules), fusionnée après validation utilisateur.
- Commits **mécaniques** (générés par script : un commit par répertoire × règle, message référençant le script et la règle) strictement séparés des commits **sémantiques** (petits, relus).
- Dès l'étape 2, `ctest -L host` vert avant chaque commit ; dès l'étape 3, `ci/run.sh` vert ; dès l'étape 5, ni régression sur la carte F439 (vérification manuelle ou exécutant CI équipé).
- Ne jamais réécrire l'historique.
- **Aucun push, aucune opération distante** (décision 2026-09-30) : toutes les opérations git restent locales (commits, branches, tags). Pousser vers GitHub exige de **demander l'accord de l'utilisateur à chaque fois**, en indiquant la branche et les commits concernés ; le hook `.claude/hooks/lepton_guard.py` force cette demande pour toute commande `git push`. La supervision se fait sur la machine locale (`MIGRATION-STATUS.md`, `handoff/`, `git log`).

## 6. MIGRATION-STATUS.md

Fourni prérempli par le paquet de passation : décisions actées, versions vérifiées, constats sur
l'arbre, décisions ouvertes, tableaux d'avancement (étapes, modules de l'étape 4, matrice du banc
KAL). Le tenir à jour sans en changer la structure ; ajouter une ligne datée à « Décisions
actées » pour chaque décision du §4.

## 7. Agents multiples — usage ciblé, pas systématique

Le plan est séquentiel et les fichiers d'étape autonomes : les sous-agents ne sont **pas nécessaires** à la correction, ils servent la volumétrie et l'hygiène de contexte. Usages recommandés :

- **Exploration/audit en lecture seule** (étape 1, vérifications) : déléguer les parcours massifs du dépôt à des sous-agents qui ne rapportent que la synthèse — le contexte principal reste propre.
- **Étape 4, transformation par répertoires** : parallélisable par sous-agents **uniquement sur des ensembles de fichiers disjoints** (un répertoire par agent), chacun appliquant les scripts et rapportant ses métriques. Fusion et validation globale (mass_compile + socle QEMU) par la session principale.
- **Étape 6, extension par carte** : une carte par agent, mêmes conditions de disjonction.
- **Agent vérificateur** : après un lot, un sous-agent rejoue build + tests et rapporte, pendant que la session principale prépare la suite.

Règles : jamais deux agents écrivant dans les mêmes fichiers ; les fichiers partagés (`MIGRATION-STATUS.md`, `compiler.h`, CMake communs) ne sont modifiés que par la session principale ; les étapes à fort couplage (3, 5, 7) restent mono-agent — le débogage d'un contexte de commutation ne se parallélise pas.

## 8. Conduite en cas d'échec

- Critère de validation en échec : diagnostiquer, corriger si dans le périmètre de la session ; sinon consigner le blocage (symptôme, hypothèses, pistes) dans `MIGRATION-STATUS.md` et s'arrêter.
- Ne jamais assouplir un critère de validation, désactiver un test, ou élargir le périmètre gelé pour « faire passer » une étape : ces changements sont des décisions utilisateur (§4).
- Trois échecs successifs sur le même point : s'arrêter et présenter le diagnostic complet à l'utilisateur plutôt que d'itérer.
