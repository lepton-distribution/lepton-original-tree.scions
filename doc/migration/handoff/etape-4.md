# Handoff étape 4 → 5

État au 2026-10-01 : bilan de l'étape 4 fait sur `migration/etape-4-bilan` ; critères tous verts,
**en attente de validation utilisateur** (point d'arrêt de fin d'étape). Détail par module :
`handoff/etape-4-<module>.md` (11 modules, procédure dans `etape-4-outillage.md`).

## Réponses aux prérequis de 5
- Étape 4 close : critères verts (ci-dessous) ; clôture effective après validation utilisateur.
- Socle QEMU vert : `ci/run.sh` vert le 2026-10-01 (outils, hôte 5/5, smoke, net, KAL hard 15/15
  et soft 11/11).
- Outils de débogage : `openocd` 0.12.0, `gdb-multiarch` 16.3, règles udev `60-openocd.rules`
  installés. **Carte non raccordée** au 2026-10-01 (aucun ST-Link par `lsusb`, pas de
  `/dev/ttyACM*`) : à brancher avant la session 5.
- BSP embOS le plus proche : `ST/STM32F429_STM32F429ZI_Nucleo` (étape 1 ; vecteur CRYP et RAM à
  adapter), décision ouverte au statut.

## Critères de l'étape 4 (vérifiés le 2026-10-01, périmètre complété)
| Critère | Résultat | Preuve |
|---|---|---|
| Zéro IAR-isme, C du périmètre actif | code Lepton **0** ; tiers 56 (D1a, justifiés) | `audit_iar.py --summary`, `audit-iar.md` |
| `mass_compile.sh` 100 % | **371/371** (348 avant complément du périmètre) | `mass-compile.md` |
| Transformations rejouables ; résiduels | scripts sous `tools/migration/` ; `residuel-etape4.md` : 0 résiduel, 17 automatiques (gardes `WIN32`, étape 6) | `residuel-etape4.md` |
| KAL décomposé ; ISA/cœur hors arch | `kal/{arch,backend}` ; **0** directive | `audit_isa_ifdef.py`, `isa-ifdef.md` |
| Socle QEMU vert | `ci/run.sh` vert | ci-dessus |

## Décisions actées pendant 4
- D1a (tiers non transformé), D2a/D3a (branches gelées extraites sous `legacy/`, supprimées à
  l'étape 6) ; décisions par module : tableau « Décisions actées » du statut.
- Bilan (2026-10-01) : `perimetre.csv` complété par script plutôt que régénéré (les projets IAR
  ne déclarent pas les fichiers de la migration) ; banc KAL : sources armv7m rangées sous
  `tests/kal/arch/armv7m/` (10 directives `OS_CPU_HAS_VFP` révélées par le complément).
- Trois dettes « étape 4 » reportées (accord utilisateur) : `ARG_LEN_MAX` tronquant sans message,
  `va_list` variadique de `vfs.c` (cause du `-m32` hôte), mise en forme multi-ligne perdue par
  `transform_iar.py`. Changements de comportement, hors portage.

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `tools/migration/perimetre_complement.py` | complète `perimetre.csv` (sources créées par la migration) ; rejouer après tout ajout ou déplacement de source, avant les audits |
| `doc/migration/perimetre.{csv,md}` | périmètre ; section « Complément de l'étape 4 » : actif 1123 f / 317 465 l |
| `doc/migration/{audit-iar,isa-ifdef,mass-compile}.*` | rapports finaux de l'étape 4 |
| `tools/migration/mass_compile.py` | profils de gabarit en tête ; STM32F4 avec mkconf Olimex P407 (HYPOTHÈSE À VALIDER `STM32F429xx`) |
| `kernel/core/kal/arch/armv7m/kal_arch_conf.h`, `cmake/cpu/*.cmake` | réglages d'ISA et de cœur (aucun à toucher pour la F439 : critère « pas de modification du noyau ») |
| `handoff/etape-4-kernel-dev.md` | pilotes STM32F4 (HAL, SPL, BSP P407) : à lire avant le BSP F439 |

## Écarts au plan et pièges découverts
- Pour l'étape 5 : le preset `nucleo-f439zi-embos` **ne se configure pas** (squelette de
  l'étape 2 : `bin/net/cgi-bin/tstpost.c`, exigé par le mkconf Olimex P407, absent ; ni BSP ni
  CMSIS device). Les pilotes STM32F4 n'ont été que compilés (mass_compile), jamais exécutés.
- Pour l'étape 5 : rootfs, `dest_path` à plusieurs niveaux (`/usr/bin/net` créé sans `/usr/bin`) :
  défaut préexistant, à vérifier sur le mkconf P407 ; `__lepton_packed` (`flash.h`) : compactage à
  vérifier ; pragma CCM F4 traité à la main (`kernel/core`).
- Audits : `perimetre.csv` ne suit pas seul l'arbre ; tout fichier ajouté ou déplacé (BSP F439,
  `ld/`, `debug/`) doit être classé par `perimetre_complement.py` (sinon : arrêt « sans règle »),
  et `REGLES` complétée.
- Axe puce : 12 directives `__tauon_cpu_device__` hors BSP (`kernelconf.h` 11, `rootfscore.h` 1),
  autorisées par le critère (axe carte), à décomposer à l'étape 6.
- Sources en Latin-1 : éditer par script (`encoding="latin-1"`), l'outil Edit les réécrit en UTF-8.
- `ctest` hors `ci/run.sh` : exporter `PYTHONDONTWRITEBYTECODE=1` (sinon `.pyc` dans le trunk).
- Graphify : jamais généré (optionnel à l'étape 1) ; pas de régénération en fin d'étape.

## Non transmis volontairement
- Comptes et décisions par module : `handoff/etape-4-*.md`, `git log --first-parent master`.
- Dette hors étape 5 (sécurité FPU/MPU, `ctype.h`, `stty.c`, `telnetd.c`, `free`) : statut,
  « Blocages et dette ».
