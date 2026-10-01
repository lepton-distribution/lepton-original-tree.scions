# Handoff étape 4, module KAL (`kernel/core/kal.h`) → modules suivants

État au 2026-10-01 : module fait sur `migration/etape-4-kal`, validé par l'utilisateur le
2026-10-01 et fusionné ; `ci/run.sh` vert. Procédure générale : `handoff/etape-4-outillage.md`.

## Résultat du module
- `kal.h` : 2137 → 71 lignes, **dispatcher sans condition** : `#include "kal_arch.h"` puis
  `"kal_backend.h"`, choisis par chemins d'inclusion (`LEPTON_KAL_ARCH_DIR` posé par
  `cmake/isa/<isa>.cmake`, `LEPTON_KAL_BACKEND_DIR` par `cmake/kal/<backend>.cmake`).
- `src/kernel/core/kal/` : `arch/armv7m` (`__va_list_copy`, SysTick, bit EXC_RETURN FPU),
  `arch/host` (`__va_list_copy` i386), `backend/embos`, `backend/static`, `backend/freertos`
  (tel quel, non compilé), `contrat.h` (ancienne branche `#else`, spécification, non incluse).
- `kal.c` supprimé (tout sous `CPU_WIN32`, compilé nulle part) ; copies d'origine `kal.h`, `kal.c`
  sous `scion/legacy/`.
- `audit_iar.py` : `kernel/core` Lepton 7 → **0** ; actif 116 → 109 (Lepton 51, tiers 58).
- `audit_isa_ifdef.py` : 68 → **51** directives hors arch (19 fichiers) ; plus rien dans `kal.h`.
- `mass_compile.sh --only sys/root/src/kernel/core` 50/50 ; périmètre 246/348 (inchangé).
- `gcc -E -P` identique (presets hôte, hard, soft, 348 commandes de mass_compile) à chaque commit
  mécanique ; deux commits sémantiques à différences vérifiées : SysTick `OS_U32` → `uint32_t`
  (22 lignes) et primitive EXC_RETURN (hard seulement) ; **code objet identique** (objdump -d, 137
  objets du preset hard). Banc KAL : 15/15 hard, 11/11 soft ; smoke, net, host verts.

## Décisions actées pendant le module (2026-10-01)
- Emplacement `kernel/core/kal/{arch,backend}/` (ETAPE-4), `kal.h` laissé en place (25 inclusions).
- Copies d'origine sous `legacy/` (comme D2a/D3a), pas `kal/legacy/`.
- Branche FreeRTOS extraite telle quelle ; ses 3 `#if` de cœur restent dans `backend/freertos`
  jusqu'à l'étape 7 (exception justifiée : non compilable aujourd'hui).

## Artefacts produits (manifeste)
| Fichier | Contenu, quand le lire |
|---|---|
| `tools/migration/kal_split.py` | décomposition rejouable : `gel`, `anciens-coeurs`, `extraire-*`, `dispatcher` |
| `tools/migration/tests/test_kal_split.py` | retraits sur fixtures + rejeu complet sur `legacy/` (ci/run.sh) |
| `scion/sys/root/src/kernel/core/kal/` | KAL par axe ; `contrat.h` = ce qu'un backend doit définir |
| `doc/migration/ajout-coeur.md` | §1, §3, §5 mis à jour (kal_arch.h / kal_backend.h) |

## Écarts au plan et pièges découverts
- `host.ufs_format_arm` (tests/host) recopie à la main les options de l'axe hôte pour compiler avec
  arm-none-eabi : il a cassé au dispatcher ; corrigé en exposant `LEPTON_KAL_*_DIR`. Tout nouveau
  test qui compile hors `lepton_options` doit reprendre ces variables.
- **Les audits ne voient pas les fichiers créés par la migration** : `perimetre.csv` (étape 1) ne
  contient ni `kal/`, ni les fichiers des étapes 2-3 (`startup_armv7m.c`, `embos_main.c`…).
  `kal/` audité ici par un périmètre temporaire : 0 hors arch, 4 autorisées (`OS_CPU_HAS_VFP` du
  backend embOS = micro-noyau × cœur ; 3 FreeRTOS). À traiter (régénérer ou compléter le périmètre).
- `kal/arch/armv6m` non créé : embOS n'a jamais eu de branche M0 ; `armv6m.cmake` n'ajoute pas de
  chemin ; une configuration armv6m échoue sur `kal_arch.h` introuvable (voulu, étape 6).
- `backend/static` garde des vestiges x86 (`context_t` esp/ebp…, `enum_synth_regs`) : déplacer en
  `arch/host` changerait l'ordre des déclarations (gcc -E) ; sans usage, laissés.
- `backend/embos` garde les branches `OS_VERSION_GENERIC` < 5.18 (versions embOS anciennes, mortes
  avec 5.20) : hors axe ISA/cœur, non retirées.
- Écart de discipline : le commit d'outil `392b920` contient aussi la suppression de `kal.c`
  (indexée par `git rm` du script avant le commit) ; historique non réécrit (ORCHESTRATION §5).
- `contrat.h` hérite d'accents perdus dans l'original (« utilis ») : non corrigé.

## Proposition pour la suite (décision utilisateur)
Session « KAL-2 » avant `kernel/dev` : les 51 directives ISA/cœur hors arch restantes, dont
`kernel/core` (`kernelconf.h` identité de cœur 12, `malloc.c` `CPU_GNU32` 8, `kernel.h` 4,
`core-segger/kernel.c` `KERNEL_STACK_SIZE` 2, `ethif_core.c` 2, `kernel_pthread.h`, `timer.h`) :
changements sémantiques (définitions CMake à la place de `__tauon_cpu_core__`, `kernel_mkconf.h`
généré). Sinon : les traiter module par module.

## Non transmis volontairement
- Détail des branches retirées : `scion/legacy/sys/root/src/kernel/core/kal.h` et diffs des
  commits mécaniques (`git log migration/etape-4-kal`).
