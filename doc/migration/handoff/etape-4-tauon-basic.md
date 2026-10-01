# Handoff étape 4, module `sys/user/tauon-basic` → modules suivants

État au 2026-10-01 : module fait sur `migration/etape-4-tauon-basic`, validé par l'utilisateur
le 2026-10-01 et fusionné. Module suivant : `tools/mklepton` (dernier module de l'étape 4). Procédure
générale : `handoff/etape-4-outillage.md`.

## Résultat du module
- Périmètre : 9 `.c` actifs (`src/bin/{dhrystone,free,sdramtest,net/cgi-bin}`) ; aucun n'est
  lié par un preset (QEMU : aucun binaire tauon-basic ; mkconf P407/F439 : seuls `tstpost`,
  `tstcgi2` ; dhrystone, free, sdramtest déclarés par le seul mkconf LM3S, différé).
- Commits mécaniques : `garde-cible-gelee` (`timers.c`, branche `WIN32`) ; `pragma-iar`
  (`sdramtest_main.c`, `EXT_RAM_REGION` en `__lepton_section`, macro sans usage).
- Commits sémantiques : `dlmalloc.c` → talon commenté (variante DLIB IAR entièrement sous
  `__IAR_SYSTEMS_ICC__`, vide sous GCC) ; `free_main.c` : code mort `__iar_dlmallinfo` retiré,
  code objet identique ; dhrystone : `<string.h>` (`dhry21b.c`), prototype `runDhrystone`.
  Copies d'origine sous `scion/legacy/sys/user/tauon-basic/src/bin/` (D2a).
- `gcc -E -P` identique : hard, soft (159/159), host (47/47), mass_compile du module (9/9, sauf
  `free_main.c`, différence attendue, code objet comparé).
- `mass_compile.sh` : tauon-basic **9/9** (7 avant) ; périmètre **348/348**.
- `audit_iar.py` : actif 92 → **60** ; code Lepton 36 → **4** (tous dans `tools/mklepton`).
- `audit_isa_ifdef.py` : **0** directive ISA/cœur hors arch dans le code Lepton actif (3 avant).
- `residuel-etape4.md` : 0 résiduel ; 17 automatiques restants = gardes `WIN32` de `kernel/core`
  et `dev_ftl.c`, reportées à l'étape 6 (décision 2026-10-01).
- `ci/run.sh` vert (outils 49 tests, hôte 5/5, smoke, net, KAL hard 15/15 et soft 11/11).

## Décisions actées pendant le module (2026-10-01, plan approuvé)
- `dlmalloc.c` : talon commenté (et non déplacement ni transformation des gardes internes),
  pour garder `perimetre.csv`, mass_compile et les `.ewp` cohérents jusqu'à l'étape 6.
- `free` : reste sans effet (comme avant) ; pas d'implémentation (le tas Lepton n'expose pas
  de statistiques : fonctionnalité nouvelle, hors plan).

## Artefacts produits (manifeste)
| Fichier | Contenu, quand le lire |
|---|---|
| `scion/legacy/sys/user/tauon-basic/src/bin/` | copies d'origine (`timers.c`, `dlmalloc.c`, `free_main.c`), gelées, supprimées à l'étape 6 |
| `doc/migration/{audit-iar,mass-compile,residuel-etape4}.*` | rapports régénérés en fin de module |

## Écarts au plan et pièges découverts
- Le tableau des modules annonçait « `dlmalloc.c` : traitement manuel (choix d'implémentation de
  `free`) » : en fait `free_main` était déjà neutralisé (`#if 0`) et dlmalloc vide sous GCC ;
  aucun allocateur n'est à choisir.
- `pragma-iar` signale `EXT_RAM` absente des `.ld` : sans objet ici (macro jamais utilisée) ;
  à reprendre seulement si une carte utilise une SDRAM externe (étape 5/6).
- Le blocage du preset `nucleo-f439zi-embos` cite `bin/net/cgi-bin/tstpost.c` absent : le fichier
  existe sous `sys/user/tauon-basic/src/bin/net/cgi-bin/` (piste non vérifiée : résolution du
  chemin des binaires applicatifs par le build ; étape 5).
- Pour `tools/mklepton` : les 4 `#pragma memory` sont dans des chaînes de génération (modèle de
  code M16C émis par mklepton), pas du code compilé ; `audit_iar.py` les classe
  `modele_mklepton`.

## Non transmis volontairement
- Dette consignée au statut : `free` sans implémentation ; `dhrystone_main.h` inclut
  `trifecta_lib.h` (absent, en-tête non inclus) ; sources dhrystone non liées par un preset.
- Détail : `git log migration/etape-4-tauon-basic`.
