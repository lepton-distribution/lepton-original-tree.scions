# Handoff étape 4, module `kernel/dev` → modules suivants

État au 2026-10-01 : module fait sur `migration/etape-4-kernel-dev` (non fusionnée, en attente de
validation). Procédure générale : `handoff/etape-4-outillage.md`.

## Résultat du module
- `mass_compile.sh --only sys/root/src/kernel/dev` : 65 → **154/154** ; périmètre 246 → **337/348**
  (sbin 32 → 34 par effet de bord d'`atof` ; aucun module en régression).
- `audit_iar.py` : `kernel/dev` Lepton 15 → **0** (tiers 16, non transformés, D1a) ; actif 109 → 94.
- `gcc -E -P` des presets hôte, hard, soft identique à chaque commit, sauf la déclaration
  d'`atof` ajoutée (aucun code) ; les autres différences ne touchent que des sources STM32F4 de
  mass_compile, vérifiées commit par commit.
- **Compilé n'est pas validé** : les pilotes STM32F4 ne sont compilés que par mass_compile (le
  preset NUCLEO ne se configure pas encore) ; validation sur carte à l'étape 5.

## Décisions actées pendant le module (2026-10-01)
- HAL ST : `cubemx_hal_driver/inc/legacy` renommé `inc/Legacy` (casse du paquet STM32CubeF4) ;
  exception étroite à D1a (nom de répertoire, contenu inchangé).

## Artefacts produits (manifeste)
| Fichier | Contenu, quand le lire |
|---|---|
| `tools/migration/mass_compile.py` | profils STM32F4 avec `mkconf` de carte (mklepton hôte → `build/mass-compile/mkconf/`) |
| `scion/legacy/…/kernel/dev/…` | copies d'origine (lm3s_cpu, BSP F4 : garde-cible-gelee, garde-compilateur) |
| `kernel/core/include/libc/{stdio,stdlib}.h` | `NULL` (POSIX) et `atof` (newlib) en freestanding |

## Écarts au plan et pièges découverts
- **Configuration de carte** : les pilotes STM32F4 génériques (`uart.c`, `gpio.c`…) reçoivent
  `UART_NB`, `_GPIO_DEFAULT_SPEED` et la puce (`STM32F407xx`) par le `user_kernel_mkconf.h` de
  l'application (via `kernelconf.h` → `kernel_mkconf.h`). mass_compile génère donc le mkconf de
  chaque carte : Olimex P407 (pilotes `arch/stm32f4xx`, base de la NUCLEO), Discovery F4,
  STM32F469I-Eval. À reprendre pour `cmake/boards/nucleo-f439zi.cmake` (étape 5).
- Le HAL CubeMX (tiers) est compilé **sans** carte (puce `STM32F429xx` du profil, HYPOTHÈSE À
  VALIDER) : avec le mkconf, sa puce et celle du profil se cumulaient (`RCC_PLLI2SInitTypeDef`
  défini deux fois).
- **Preset `nucleo-f439zi-embos` : la configuration échoue** (`bin/net/cgi-bin/tstpost.c` exigé
  par le mkconf Olimex, absent de l'arbre) ; non traité (étape 5).
- `gpio_startup_init` : chaque BSP F4 définissait une fonction `static` homonyme de la fonction
  globale de `gpio.c` ; renommée `<carte>_gpio_startup_init` (comportement IAR : appel local).
  `discovery_f4-baseboard-modem` (différé) a le même motif, non traité.
- `stm32f4xx/types.h` : `s32`/`u32` deviennent `int32_t`/`uint32_t` (long sous newlib) ; même
  taille, mais `%d` sur un `s32` ou `int*`/`s32*` peuvent donner des avertissements ailleurs.
- `.gitignore` hérité : `git add` sur `kernel/dev/arch/all/debug/…` (motif `[Dd]ebug/`) renvoie une
  erreur même pour un fichier suivi (le fichier est pourtant indexé) : committer avec un chemin
  explicite, vérifier `git status`.
- Hook `lepton_guard.py` : un `<…>` dans une commande lancée depuis le trunk est lu comme une
  redirection ; lancer depuis le clone ou écrire le script dans le scratchpad.
- `grep -r` dans le trunk ne suit pas les liens : utiliser `grep -R`.

## Non transmis volontairement
- Détail des transformations : diffs des commits (`git log migration/etape-4-kernel-dev`) et
  `residuel-etape4.md` régénéré.
