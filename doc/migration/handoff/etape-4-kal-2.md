# Handoff étape 4, module KAL-2 (directives ISA/cœur de `kernel/core`) → modules suivants

État au 2026-10-01 : module fait sur `migration/etape-4-kal-2`, validé par l'utilisateur le
2026-10-01 et fusionné. Procédure générale : `handoff/etape-4-outillage.md` ; KAL : `handoff/etape-4-kal.md`.

## Résultat du module
- `audit_isa_ifdef.py` : `kernel/core` 30 → **0** directive ISA/cœur hors arch ; total 51 → **21**
  (12 fichiers, tous hors `kernel/core` : `fs` 9, `lib/libc` 5, `sbin` 4, `tauon-basic` 3).
- `kernelconf.h`, `kernel.h`, `malloc.c`, `timer.h`, `kernel_pthread.h`, `ethif_core.c`,
  `core-segger/kernel.c` ne testent plus `__tauon_cpu_core__`, `CPU_GNU32` ni `CPU_CORTEXM`.
- `gcc -E -P` identique (hôte, hard, soft, 348 commandes de mass_compile) à chaque commit, sauf
  la règle `inutilise` : seules disparaissent les déclarations `do_swi` / `_kernel_syscall_handler`
  (jamais définies). **Code objet identique** à l'avant-module : 49 objets hôte, 160 hard, 160 soft.
- `mass_compile.sh --only sys/root/src/kernel/core` 50/50.

## Décisions actées pendant le module (2026-10-01)
- Périmètre KAL-2 = `kernel/core` seul ; les 21 autres directives dans leurs modules.
- D3a étendu à la simulation Linux (`CPU_GNU32` hors noyau statique) : retrait, copie `legacy/`.
- Réglages d'ISA dans `kal/arch/<isa>/kal_arch_conf.h` ; réglages de cœur (`__KERNEL_CPU_NAME`,
  `__KERNEL_STACK_SIZE`) en définitions de `cmake/cpu/<cœur>.cmake` (hôte : `cmake/isa/host.cmake`).

## Artefacts produits (manifeste)
| Fichier | Contenu, quand le lire |
|---|---|
| `tools/migration/axes_kernel_core.py` | règles `statique`, `gelee`, `inutilise` (atomiques, Latin-1 octet pour octet) |
| `tools/migration/tests/test_axes_kernel_core.py` | primitives `keep_first_arm`, `replace_exact` |
| `kal/arch/{host,armv7m,armv6m}/kal_arch_conf.h` | support 64 bits, largeur, profil (hôte), signaux RT, verrous |
| `cmake/cpu/*.cmake`, `cmake/isa/host.cmake` | `__KERNEL_CPU_NAME`, `__KERNEL_STACK_SIZE` |
| `scion/legacy/…/kernel/core/timer.h` | seule nouvelle copie d'origine (les autres existaient) |

## Règles réutilisables pour les modules suivants
- `CPU_GNU32` signifie presque toujours « noyau statique hôte » : remplacer par
  `USE_KERNEL_STATIC` quand la condition porte sur le micro-noyau (contenu constant : CPU_GNU32 n'est
  posé qu'avec USE_KERNEL_STATIC) ; sinon c'est la simulation Linux, gelée.
- `CPU_CORTEXM` sans `__KERNEL_UCORE_*` = eCos Cortex-M, gelé.

## Écarts au plan et pièges découverts
- **Profil du noyau** : sur cible, `__tauon_kernel_profile__` vient du `kernel_mkconf.h` généré
  par mklepton ; un `#define` inconditionnel dans `kal_arch_conf.h` l'écrasait (tubes 10 → 1,
  détecté par gcc -E). Corrigé : seul l'hôte force `full`. Conséquence : le forçage « minimal » du
  **seul Cortex-M3** (kernelconf.h d'origine) disparaît ; à revoir à l'étape 6 si une carte M3 en
  dépend (valeur d'origine dans `legacy/sys/root/src/kernel/core/kernelconf.h`).
- `kernelconf.h` est en Latin-1 mais le bloc ajouté à l'étape 2 (`kernel_mkconf.h`) était en UTF-8 :
  fichier à encodage mixte ; le script compare ce bloc sous sa forme UTF-8 et écrit de l'ASCII.
- Première version de `gelee` non atomique (fichier écrit avant l'échec d'un autre) : corrigée
  (`commit_all`) ; état partiel annulé avant tout commit.
- `kal/arch/armv6m/kal_arch_conf.h` existe (valeurs M0 de l'IAR, HYPOTHÈSE À VALIDER) mais pas
  `kal_arch.h` : armv6m reste non compilable (étape 6).
- Tests compilant hors `lepton_options` (`host.ufs_format_arm`) : `__KERNEL_CPU_NAME` y vaut
  « unknow » (repli de kernelconf.h), sans effet sur ce contrôle de syntaxe.
- `perimetre.csv` ne couvre toujours pas les fichiers créés par la migration (`kal/` audité à part :
  0 hors arch ; dette consignée au module KAL).

## Non transmis volontairement
- Détail des branches retirées : copies `legacy/` et diffs des commits mécaniques
  (`git log migration/etape-4-kal-2`).
