# Handoff étape 1 → 2

## Réponses aux prérequis de 2
- Inventaire : `inventaire-projets.md` (46 `.ewp`, 4 générations), `perimetre.md` (actif / différé /
  gelé, sources dérivées `mps2-an386` 217 f et NUCLEO-F439ZI 353 f).
- Classement par axe (ISA / cœur / carte / micro-noyau) : `cartographie-kal.md`.
- Noyau statique : `noyau-statique.md` (+ `noyau-statique-sources.csv`) — sources `x86_static`,
  absents, dépendances à couper (fichier:ligne), structures de l'image UFS et tailles hôte/ARM.
- mklepton : `mklepton.md` — portage natif faisable, pas de repli sur les sources.
- Sorties de référence : `mklepton-ref.md` (voie de repli décidée, voir ci-dessous).
- Trunk : décision étape 0 = chemins mkconf rendus relatifs au trunk ; sorties hors trunk.
- Outils : CMake 3.31.6, Ninja 1.12.1, gcc 14.2, `libexpat1-dev` 2.8.3 installés (2026-09-30).
- Graphe : `dependances.md` / `.dot` (proposition de découpage en bibliothèques, §7).

## Décisions actées pendant 1
- 2026-09-30 : licence embOS SFL, usage évaluation / non commercial ; paquet V5.20.0.0 hors git
  dans `~/lepton/third_party/embos/cortexm-gcc/5.20.0.0/` (`LEPTON_EMBOS_ROOT`).
- 2026-09-30 : code gelé = proposition `code-gele.md` acceptée sauf SAMV71/SAME70 (différé,
  référence M7). Gelé : 765 f / 158 061 l.
- 2026-09-30 : oracle mklepton sans binaire (`libkernel.so` i386 absente ; copie historique hors
  arbre non utilisée) ; image UFS validée par exécution sous QEMU (étape 3).

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu et quand le lire |
|---|---|
| `perimetre.md`, `perimetre.csv` | ensembles par fichier, listes de sources dérivées ; pour écrire les listes CMake |
| `inventaire-projets.md`, `matrice-fichiers-projets.csv` | defines/includes/cpu par projet IAR ; pour les options CMake |
| `dependances.md` / `.dot` | cycles et découpage ; pour les bibliothèques statiques |
| `noyau-statique.md` | cible du noyau statique hôte ; tâche noyau statique |
| `mklepton.md`, `mklepton-ref.md` | portage natif, entrées/sorties, référence ; tâche mklepton |
| `cartographie-kal.md` | axes de variation, `#if` hors arch ; pour `cmake/cpu`, `cmake/boards` |
| `embos-inventaire.md`, `embos-api.txt`, `embos-iar-vs-gcc.md` | variantes de libs ↔ flags, BSP F429ZI, écarts E1-E7 ; étapes 2-3 |
| `audit-iar.md` / `.csv` | IAR-ismes (actif 148) ; métrique des étapes 3-4 |
| `code-gele.md` | liste gelée décidée |
| `tools/migration/*.py`, `*.sh` | scripts rejouables (commandes en tête de chaque rapport) |
| `$LEPTON_BUILD/etape-1/` | intermédiaires (JSON projets, objets depgraph, stubs) ; hors git |

## Écarts au plan et pièges découverts
- `kal.h` : branche embOS réservée IAR/Keil (E1, bloquant sous GCC) ; `|| cortexM7` hors
  parenthèses ; `RTOS.H` en majuscules ; `OS_MakeTaskReady` non déclaré (GCC 14).
- `__compiler_directive__packed` vide sous GCC (3 usages actifs) ; `etypes.h` : `int64_t` = `long`.
- Noyau statique : compilation freestanding obligatoire (conflits avec la glibc) ; 39/45 fichiers
  passent ; mécanisme `_kernel_in_static_mode` déjà présent (`kernel.h:179-197`).
- UFS : nœud 24 o sous GCC (hôte et ARM identiques), 18 o sous MSVC/IAR `pack(1)` ; `cmtime` =
  horloge hôte (masquer ou `SOURCE_DATE_EPOCH`).
- mkconf : `dest_path` absolus d'anciens postes ou `$(HOME)/tauon` ; 229 chemins `c:/tauon/…`.
- Cycle principal (15 composants, 376 symboles) : core ↔ core-segger ↔ vfs ↔ libc (7 symboles
  retour) → `LINK_GROUP:RESCAN` nécessaire au départ.
- Aucun projet STM32F7 ; `kernel_mkconf.h` absent de l'arbre (sortie mklepton) : stub utilisé par
  `dep_graph.py` (`HYPOTHÈSE À VALIDER`).
- Hook `lepton_guard.py` : tout `>` d'une commande (même dans un message `-m`) est lu comme une
  redirection relative au cwd de session → message de commit par `-F <fichier>`.
- Du code Segger embOS IAR est versionné sous `ucore/embOS*` : point de licence avant tout push.

## Non transmis volontairement
- Détail des hypothèses des sous-rapports (listées en fin de chaque document).
- Graphe Graphify (optionnel) : non produit.
- Décisions étape 1 encore utiles plus tard : premier palier soft-float (`libosT7LSP.a`) ou
  hard-float — à trancher à l'étape 3 (`embos-inventaire.md`).
