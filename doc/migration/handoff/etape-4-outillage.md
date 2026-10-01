# Handoff étape 4, session 4.0 (outillage, ligne de base) → modules de l'étape 4

État au 2026-10-01 : outillage livré sur `migration/etape-4-outillage` ; **aucun source Lepton
transformé**. `ci/run.sh` vert. Les modules suivants partent de cette ligne de base.

## Réponses aux prérequis des modules
- Tâche 1 (`compiler.h`) : déjà complète depuis l'étape 3 (8 macros de la table ; `.noinit` et
  `.ramfunc` dans `ld/common-cortexm.ld`) : rien à faire.
- Tâche 2 (règles) : `transform_iar.py` couvre tous les IAR-ismes du code Lepton actif : règles
  `garde-iar-gelee`, `garde-iar-arm`, `intrinsics-cmsis`, `mot-cle-iar`, `pragma-iar`,
  `header-iar`, `symbole-iar` (en-tête du script). Simulation sur le périmètre actif :
  15 fichiers, 40 automatiques, 34 résiduels → `residuel-etape4.md`.
- Tâche 4 (tableau de bord) : `mass_compile.sh` : **243/348 (69,8 %)** ; par module dans
  `mass-compile.md` (et MIGRATION-STATUS, tableau des modules).
- Tâche 3 (KAL) : inventaire `isa-ifdef.md` : 101 directives ISA/cœur hors arch, 29 fichiers
  (dont `kernelconf.h` 21, `kal.h` 17) ; 42 directives « puce » seule hors BSP.

## Décisions actées pendant 4.0
- D1a : code tiers non modifié ; ses 58 IAR-ismes justifiés (section de `residuel-etape4.md`) ;
  critère « zéro IAR-isme » = code Lepton actif (`audit_iar.py` : « code Lepton »).
- D2a : branches gelées des fichiers actifs extraites à contenu constant, supprimées à l'étape 6.
  Mise en œuvre outillée : `garde-iar-gelee` copie le fichier d'origine à l'identique sous
  `scion/legacy/<chemin>` (non compilé, à ajouter à `code-gele.md` au premier usage) avant de
  retirer les branches M16C. Pour `kal.h`, la session KAL extrait les branches gelées
  (eCos, ARM7/9, M16C, Win32) vers `kal/legacy/` comme proposé, ou réutilise `legacy/` : à fixer
  au plan de cette session.

## Artefacts produits (manifeste)
| Fichier | Contenu, quand le lire |
|---|---|
| `tools/migration/transform_iar.py` | règles ; `--perimetre-actif`, `--rule`, `--apply`, `--report`, `--tiers-from-audit`, `--legacy-dir`, `--cpp-snapshot/--cpp-compare` |
| `tools/migration/tests/test_transform_iar.py` | 28 tests (ci/run.sh) ; ajouter un test par nouvelle règle |
| `tools/migration/mass_compile.sh` / `.py` | compilation de masse ; `--only <préfixe>` pour un module ; profils de gabarit en tête |
| `tools/migration/audit_isa_ifdef.py` | inventaire ISA/cœur/puce ; `--summary` pour le critère |
| `doc/migration/mass-compile.md`, `.csv` | ligne de base par module, histogramme, échecs (première erreur) |
| `doc/migration/residuel-etape4.md` | rapport de simulation (régénérable) : résiduels, automatiques, tiers justifiés |
| `doc/migration/isa-ifdef.md`, `.csv` | directives d'axe hors arch, par fichier |
| `doc/migration/audit-iar.md`, `.csv` | audit régénéré le 2026-10-01 (actif 128 : Lepton 70, tiers 58) |

## Procédure d'un module (rappel opérationnel)
1. `transform_iar.py --cpp-snapshot <compile_commands> avant.json` (presets hard, soft, host).
2. Par règle : `transform_iar.py --rule <r> --files-from <liste du module> --apply`, puis
   `scion graft`, contrôle `find "$LEPTON_TRUNK" -type f ! -name .scion.grafted.list` vide,
   commit mécanique « <module> — règle <r> (transform_iar.py) ».
3. `--cpp-compare` : identique attendu pour les gardes ; différences attendues et vérifiées pour
   `mot-cle-iar`/`pragma-iar` (attributs).
4. Résiduels du module : commits sémantiques séparés ; `mass_compile.sh --only` ; `ci/run.sh`.

## Écarts au plan et pièges découverts
- Inventaire ISA/cœur : 101/29 fichiers contre « 161/34 » relevé à l'étape 1 (méthode non
  conservée) ; la référence est désormais `audit_isa_ifdef.py`.
- Preset `nucleo-f439zi-embos` = squelette de l'étape 2 (ni BSP ni CMSIS device) : mass_compile
  utilise un profil STM32F4 sur gabarit QEMU (`STM32F429xx`, `USE_STDPERIPH_DRIVER`,
  HYPOTHÈSE À VALIDER, étape 5).
- Erreur dominante (71 fichiers, `kernel/dev`) : `Legacy/stm32_hal_legacy.h` introuvable :
  casse Windows dans le HAL ST (tiers ; répertoire `inc/legacy`). Non modifiable (D1a) : à
  résoudre côté build au module `kernel/dev` (chemin d'inclusion ou lien `Legacy` généré dans
  `$LEPTON_BUILD`).
- `__packed union` (`stm32f4xx/types.h`) : 13 échecs en cascade, traités par `mot-cle-iar`.
- Autres échecs = erreurs GCC 14 réelles (déclarations implicites, `static` après déclaration,
  pointeurs incompatibles) : corrections sémantiques par module, pas des IAR-ismes.
- `dlmalloc.c` (tauon-basic) est la variante DLIB d'IAR, entièrement sous `__IAR_SYSTEMS_ICC__` ;
  `free` appelle `__iar_dlmallinfo` : traitement manuel (choix d'implémentation de `free`).
- `kernelconf.h` : branche IAR CCM (`_Pragma location`) résiduelle (étape 5, section absente
  des .ld) ; `#define __compiler_directive__packed __packed` est dans la branche **Keil**
  (catégorie « autre ») mais compté IAR par l'audit.
- `#pragma anon_unions` (ARMCC) dans `stm32f4xx/gpio.c` : non IAR, à traiter au module dev.
- `ci/run.sh` : les tests Python écrivaient `trunk/tests/__pycache__/*.pyc` (fichier régulier
  dans le trunk) ; corrigé par `PYTHONDONTWRITEBYTECODE=1`. Un `ctest` lancé à la main peut
  encore en créer : exporter la variable. Lien greffé résiduel vers
  `scion/tests/__pycache__/net_qemu…pyc` (ignoré par git) : nettoyage = `graft-clean` (décision).

## Non transmis volontairement
- Détail des commandes de chaque fichier : `mass-compile.csv` (colonne `commande`) et le script.
