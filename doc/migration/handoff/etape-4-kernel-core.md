# Handoff étape 4, module `kernel/core` (hors KAL) → modules suivants

État au 2026-10-01 : module fait sur `migration/etape-4-kernel-core` (non fusionnée) ; `ci/run.sh`
vert ; `kal.h` exclu (session KAL). Procédure générale : `handoff/etape-4-outillage.md`.

## Résultat du module
- `mass_compile.sh --only sys/root/src/kernel/core` : **50/50** (47 avant) ; périmètre 246/348.
- `audit_iar.py` : `kernel/core` Lepton 19 → **7, tous dans `kal.h`** ; actif Lepton 70 → 58.
- `audit_isa_ifdef.py` : 101 → 68 directives ISA/cœur hors arch (20 fichiers). Restent dans
  `kernel/core` (cibles **actives**, à déplacer par la session KAL vers `arch/`) : `kernelconf.h`
  (1 isa, 11 cœur), `malloc.c` (8 × `CPU_GNU32`), `kernel.h` (4), `core-segger/kernel.c` (2 cœur),
  `ethif_core.c` (2), `kernel_pthread.h` (1), `timer.h` (1).
- `gcc -E` identique avant/après pour les quatre règles de gardes (presets hard, soft, hôte et
  commandes de mass_compile, 348 sources) ; seuls les 3 fichiers corrigés exprès diffèrent.

## Décisions actées pendant le module
- D3a (2026-10-01) : branches des cibles gelées (Win32, ARM7/ARM9, M16C, eCos) et des compilateurs
  non GCC (Keil, Visual C) retirées avec copie d'origine sous `legacy/` ; gardes GCC levées.

## Artefacts produits (manifeste)
| Fichier | Contenu, quand le lire |
|---|---|
| `tools/migration/transform_iar.py` | + `garde-cible-gelee` (ex-`garde-iar-gelee`), `garde-compilateur`, `prototype-static` ; CRLF conservés |
| `tools/migration/mass_compile.py` | + `--db-out` : commandes au format compile_commands.json (pour `--cpp-snapshot`) |
| `tools/migration/audit_iar.py` | `legacy/` classé gelé |
| `scion/legacy/sys/root/src/kernel/core/…` | 24 copies d'origine (gelées) ; `code-gele.md`, dernière section |
| `doc/migration/residuel-etape4.md` | simulation sur tout le périmètre actif après ce module (26 résiduels) |

## Procédure appliquée (modèle pour les modules suivants)
1. Liste du module : `perimetre.csv` actif, hors tiers (et hors `kal.h` ici) → fichier liste.
2. Instantanés : `--cpp-snapshot` des 3 presets + `mass_compile.sh --db-out` puis `--cpp-snapshot`.
3. Par règle (ordre : `garde-cible-gelee`, `garde-iar-arm`, `garde-compilateur`, puis les autres) :
   `--apply`, `scion graft`, contrôle du trunk, `--cpp-compare`, commit mécanique « <module> — règle
   <r> (mécanique) ». **Appliquer et committer une règle à la fois.**
4. Corrections manuelles en commits sémantiques ; `mass_compile.sh --only` ; `ci/run.sh`.

## Écarts au plan et pièges découverts
- Erreurs GCC 14 « static après déclaration non static » : motif répété (24 occurrences) → règle
  `prototype-static` plutôt que correction à la main ; le même motif existe dans `kernel/dev`
  (`dev_rtc_nxp_pca8565.c`, `dev_ppp_uip.c`) et ailleurs : relancer la règle par module. Prototype
  dans un en-tête (`modem_core.h`) : hors règle, `static` retiré de la définition à la main.
- **Encodage** : les sources sont en Latin-1. L'outil Edit de Claude Code réécrit le fichier en
  UTF-8 et remplace les caractères Latin-1 existants par U+FFFD (constaté sur `kernelconf.h`, annulé).
  Éditer ces fichiers par script Python (`encoding="latin-1", newline=""`), jamais avec Edit/Write.
- Le hook `lepton_guard.py` lit un `>` d'un heredoc Python comme une redirection si le répertoire
  courant est dans le trunk : lancer les scripts depuis le clone ou depuis un fichier du scratchpad.
- `gcc -E` des sources héritées : sortie en Latin-1 (`cpp_outputs` décode en Latin-1).
- Branches `defined(__GNUC__)` levées sans correction : celles qui viennent de la simulation Linux
  peuvent porter des valeurs fausses sur cible (handoff de l'étape 3) : `dirent.c`, `stat.c`,
  `kernel_pthread.h`, `select.h`, `kernelconf.h` (originaux sous `legacy/`), à revoir si un test
  échoue, pas à corriger en passant.
- `kernelconf.h` : placement CCM (IAR) retiré ; à reprendre à l'étape 5 si la F439 en a besoin.

## Non transmis volontairement
- Liste détaillée des branches retirées : diff des commits mécaniques et copies `legacy/`.
