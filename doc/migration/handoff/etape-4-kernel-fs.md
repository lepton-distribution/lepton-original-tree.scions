# Handoff étape 4, module `kernel/fs` → modules suivants

État au 2026-10-01 : module fait sur `migration/etape-4-kernel-fs`, validé par l'utilisateur
le 2026-10-01 et fusionné. Procédure générale : `handoff/etape-4-outillage.md`.

## Résultat du module
- `mass_compile.sh --only sys/root/src/kernel/fs` : 23 → **26/26** ; périmètre 337 → **340/348**.
- `audit_isa_ifdef.py` : `kernel/fs` 9 → **0** ; total 21 → **12** (6 fichiers : `lib/libc` 5,
  `sbin` 4, `tauon-basic` 3).
- `audit_iar.py` : `kernel/fs` Lepton 2 → **0** (après reclassement) ; tiers 2 (`diskio.c` résolu
  par `ffconf.h`, `yportenv.h`).
- `gcc -E -P` identique à chaque commit sur les presets hôte, hard, soft, sauf la déclaration de
  `strtok_r` (aucun code) ; mass_compile : différences vérifiées (`diskio.c`, `fatfscore.c`).

## Décisions actées pendant le module (2026-10-01)
- FatFs `ffconf.h` (configuration, tiers) : `__weak` défini par `compiler.h` (`__lepton_weak`),
  exception étroite à D1a.
- `fatfscore.c` : `f_close(&root_dir)` corrigé en `f_closedir` (correctif de comportement).

## Artefacts produits (manifeste)
| Fichier | Contenu, quand le lire |
|---|---|
| `tools/migration/axes_kernel_fs.py` | règles `statique`, `gelee`, `doublon` (s'appuie sur `axes_kernel_core.py`) |
| `tools/migration/audit_iar.py` | `ORIGINE_TIERS` : `fatfs/core` seul (glue `fatfs.c`, `fatfscore.c` = Lepton) |
| `kal/arch/<isa>/kal_arch_conf.h` | + `__KERNEL_MAX_SUPER_BLOCK` (hôte 8, Cortex-M 4) |
| `kernel/core/include/libc/string.h` | + `strtok_r` |

## Écarts au plan et pièges découverts
- Le reclassement de `fatfs/` change la ventilation Lepton/tiers des audits (Lepton 36 → 38 avant
  transformation) sans changer le total.
- `ORIGINE_TIERS` reste large ailleurs (`lwip`, `uip`, `yaffs`, `mongoose`…) : vérifier la licence
  d'en-tête avant de traiter un IAR-isme « tiers » d'un module (`kernel/net` en particulier).
- FAT n'est pas compilé sur l'hôte : `CPU_GNU32` y désignait la simulation Linux (gelée), alors que
  dans `rootfs`, `ufs`, `vfs` (compilés sur l'hôte) il signifie noyau statique.
- FatFs (et son correctif `f_closedir`) n'est compilé que par mass_compile : non exécuté avant
  l'étape 5.

## Non transmis volontairement
- Détail : diffs des commits (`git log migration/etape-4-kernel-fs`), copies `legacy/`.
