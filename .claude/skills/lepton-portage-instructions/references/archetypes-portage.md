# Archétypes et qualité de rédaction — contexte portage Lepton

Sommaire :
1. Archétype A — Migration (iso-fonctionnel)
2. Archétype B — Portage nouveau (création)
3. Archétype C — Infrastructure / outillage
4. Formulations efficaces vs inefficaces
5. Non-dits structurants d'un portage embarqué

---

## 1. Archétype A — Migration (iso-fonctionnel)

**Signaux :** « migrer », « porter de X vers Y », « remplacer la toolchain », un existant qui fait
foi (build IAR, backend embOS), un comportement à préserver à l'identique.

**Ce qui doit être renforcé :**

- **Oracle explicite** : identifier la référence de comparaison disponible (exécution sous QEMU,
  sorties de référence d'un outil, comportement d'un backend existant, suite de tests jouée sur les
  deux versions) et en faire des critères d'acceptation. Pour Lepton, IAR n'est pas un oracle.
- **Périmètre** : borner au périmètre actif (matrice fichier × cible) ; le code gelé est nommé ;
  critère « aucune modification hors périmètre, vérifiable par `git diff` ».
- **Transformation outillée** : sur ce dépôt (~1 MLOC), toute modification de masse passe par un
  script rejouable ; l'édition manuelle est réservée à la liste résiduelle produite par
  l'outillage. Commits mécaniques séparés des commits sémantiques.
- **Non-régression continue** : le socle QEMU (test de fumée, banc KAL) reste vert à chaque commit.

**Piège principal :** la refactorisation d'opportunité. Un agent qui traverse du code ancien tend
à l'« améliorer » ; chaque amélioration non demandée est un risque de régression et un bruit qui
rend la revue impossible. Consigne type : « traduire à l'identique, instruction par instruction ;
consigner les comportements douteux dans `dette-technique.md`, n'y toucher que si bloquant ».

**Piège secondaire :** le périmètre annoncé peut être techniquement intenable (deux fichiers
autorisés dépendant d'un mécanisme commun dans un troisième). Ne pas trancher : décision à
arbitrer.

---

## 2. Archétype B — Portage nouveau (création)

**Signaux :** « ajouter le support de », architecture ou backend inexistant (RISC-V, FreeRTOS,
TrustZone), aucun oracle direct — mais une implémentation homologue qui sert de référence
(port Cortex-M pour le RISC-V, backend embOS pour FreeRTOS).

**Ce qui doit être renforcé :**

- **Choix préalables explicites** : les décisions structurantes (MCU cible, micro-noyau, mécanisme
  d'IT, frontière libc) sont listées en tête de fichier et actées par l'utilisateur *avant* les
  tâches — jamais prises d'office dans le corps du fichier.
- **Jalons à critère de sortie vérifiable** : réutiliser les paliers existants (reset→main, init
  mémoire, warmup, appel système tracé, multitâche, E/S, endurance) plutôt qu'en inventer.
- **Parité avec l'homologue** : la colonne correspondante du banc KAL (T0-T11) jouée sur la
  nouvelle combinaison, comparée test à test à la combinaison de référence — c'est le critère
  d'iso-comportement objectif.
- **Test de propreté des abstractions** : « zéro modification hors des répertoires dédiés
  (`src/kal/arch/`, `backend/`) ; toute modification ailleurs révèle une fuite d'abstraction à
  corriger côté KAL ».

**Piège principal :** l'agent invente les formats et conventions (empilement de contexte, offsets
TCB) parce qu'il n'a pas lu la documentation du port livré. D'où l'obligation d'une phase de
lecture avec livrable (rapport d'écart) et point d'arrêt avant le code.

---

## 3. Archétype C — Infrastructure / outillage

**Signaux :** CI, banc de test, image de build, scripts d'installation, documentation, skill.

**Ce qui doit être renforcé :**

- **Reproductibilité** : versions épinglées, un seul point de vérité (ex. `install-debian.sh`
  partagé entre Dockerfile, CI et machine native), exécutable localement sans l'orchestrateur CI.
- **Intégration à l'existant** : l'outillage nouveau s'insère dans les mécanismes en place
  (CTest, labels, `MIGRATION-STATUS.md`) au lieu d'en créer de parallèles.
- **Critère d'usage réel** : « un nouveau développeur builde et flashe la cible pilote en suivant
  `BUILDING.md` sans aide », pas « la documentation est complète ».

**Piège principal :** l'outillage qui dérive en projet propre. Borner : l'outillage sert les
étapes, il n'a pas de feuille de route autonome.

---

## 4. Formulations efficaces vs inefficaces

| Inefficace | Efficace | Pourquoi |
|---|---|---|
| « Porter proprement l'assembleur » | « Traduire les directives selon la table IAR→GNU as ; ne pas modifier les instructions ; `.thumb_func` avant chaque fonction référencée par pointeur » | Vérifiable, borné |
| « Vérifier que le contexte est bien sauvegardé » | « T1 : remplir les registres de motifs connus, `__bckup_context`, corrompre, restaurer, vérifier motif par motif ; variante FPU sur M4/M7 » | Procédure exécutable |
| « Faire attention aux différences embOS » | « Confronter tout dysfonctionnement des paliers 4-5 à `embos-iar-vs-gcc.md` avant d'investiguer ailleurs » | Conduite à tenir |
| « Ne pas casser le build » | « `ctest -L smoke` et `ctest -L kal` verts sur `mps2-an386` à chaque commit » | Contrôlable |
| « Bien gérer le gros volume de code » | « Périmètre actif uniquement ; un commit par (répertoire × règle) ; relecture du commit mécanique = relecture de la règle + sondage » | Change le comportement |
| « S'inspirer du port Cortex-M » | « Reprendre du port Cortex-M : structure des paliers, harnais de test, nommage des symboles `.ld`. Le port Cortex-M est en lecture seule. » | Précise quoi reprendre, interdit d'y toucher |

**Règle de relecture :** pour chaque ligne, « si je la supprime, l'agent agira-t-il différemment ? »
Si non, supprimer.

---

## 5. Non-dits structurants d'un portage embarqué

À chercher systématiquement dans la source ; chaque absent devient une question ou un
`<À CONFIRMER>`. Ne remonter que ceux qui bloquent réellement le travail décrit.

**Toolchain et binaire**
- versions exactes (compilateur, binutils, newlib, micro-noyau) et politique d'épinglage
- flags CPU/FPU/ABI (`-mfloat-abi`, `-march`) et correspondance avec les bibliothèques liées
- convention d'empilement de contexte, offsets de structures partagées C/asm
- sections spéciales (`.noinit`, `.ramfunc`, vecteurs, config words à adresse fixe)

**Périmètre**
- quelles cibles/variantes sont réellement actives vs héritées
- droit de modifier les interfaces partagées (KAL, compiler.h, CMake communs)
- sort du code gelé ; rétrocompatibilité attendue ou non
- la baseline de comparaison existe-t-elle encore et jusqu'à quand

**Validation**
- oracle disponible (baseline, homologue, suite de tests) et procédure de comparaison
- matériel réel : lequel, avec quelle sonde, qui l'a sur table
- ce que QEMU couvre et ne couvre pas pour ce chantier
- seuils d'occupation flash/RAM acceptables

**Opérationnel**
- opérations irréversibles (écriture flash/OTP, effacement)
- licences (embOS, IAR résiduel) et contraintes de redistribution/vendoring
- qui arbitre, et points d'arrêt attendus
