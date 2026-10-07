# Handoff étape 8 (hôte 64 bits, retrait de `-m32`) → étape 9 (hôte macOS)

État au 2026-10-07 : branche `migration/etape-8` (locale, non fusionnée), tâches 1 à 6 faites,
critères verts, **en attente de validation utilisateur**. Debian 13, gcc 14.2.0, clang 19.1.7.

## Réponses aux prérequis de l'étape 9
- Preset `host` en 64 bits : `file $LEPTON_BUILD/host/mklepton` → « ELF 64-bit LSB pie
  executable, x86-64 ». `ctest -L host` 6/6 avec gcc et avec clang.
- `scion/tests/host/mklepton_empreintes.sha256` versionné (commit `03cd0d7`, seul commit du
  fichier) : 38 fichiers produits par mklepton `-m32` ; identiques en 64 bits avec gcc et clang.
  Test permanent `host.mklepton_empreintes` (label `host`).
- Build clang vert sur Debian, intégré à `ci/run.sh` (`$LEPTON_BUILD/host-clang`).
- Feuilles du trunk : **4019** (`scion graft` ; `scion rootstock-information` : 2 scions greffés).
- SHA-256 des `.bin` des 14 presets : **à comparer seulement à date fixe**, obtenus par
  `SOURCE_DATE_EPOCH=0 ci/run.sh` après `cmake --build <dir> --target clean` des presets (vérifié).
  Valeurs Debian (identiques avant et après l'étape) :

| Preset | SHA-256 de `lepton.bin` (`SOURCE_DATE_EPOCH=0`) |
|---|---|
| nucleo-f439zi-embos | `6ff04e485d68ae3cfd273699630c3f11b3a411ec50ccdefaea2eb85b0944841d` |
| nucleo-f439zi-freertos | `b6e9aae77c29f0358d963fad13d8e9b994f1ee5513dabfee13f90ad6133cbff3` |
| nucleo-wl55jc1-embos | `308d4e757ae3ce40600ea60c4ab107fb03b1da621438bef81bbee40f3b1c1612` |
| nucleo-wl55jc1-freertos | `e2743fcc6ebc5955df68a4ac1fe5888422c4f62cf73b7e57b3a4b5232d4b851c` |
| qemu-mps2-an386-embos | `9609a9dec6bd0ac7470ef0f76bc24905befcbf0a4af940b32279409240af650a` |
| qemu-mps2-an386-embos-soft | `880f84c9fe6b0c41f7a525c1f5cf29db55adba6941bdfef3c006700f681db4ed` |
| qemu-mps2-an386-freertos | `951af551f16bfa0e9c293133973486c866c9c4abc3125846dd6fcda8b3db5231` |
| qemu-mps2-an386-freertos-soft | `9d95a97f468ce81e2a48da75d8609296dd5b849b51d2d99a0bb03c957cfc787a` |
| qemu-mps2-an500-embos | `66de02a4baa498b366962d96e12c88954456a9ad3be90b49eba7b3901e7afb8c` |
| qemu-mps2-an500-freertos | `5b62677a7d5893f85b9ffcd5f8017da31b609ce210310123f483759a063a6b71` |
| samd21-xplained-pro-embos | `db2edf804e38cd7c3a406ec8df4794fcf8e5fac773efd68e21e1445292ce0311` |
| samd21-xplained-pro-freertos | `a323efd4b76cb6d67585d5e4457e1f2d506841576f1aeaad3b48a20f71cb91c1` |
| stm32f746g-disco-embos | `7d463bc98627aac21a1a8c5b3c96e56fa548e511f431672cbc4635cac55d11fd` |
| stm32f746g-disco-freertos | `feeaf29a4e3d02f7bc39fa500ef530711f424f0298246abcd14e6f6f50b20ba8` |

## Décisions actées pendant l'étape 8
- Appliquées : macro d'architecture `__va_list_from_arg` et retrait de `-m32` (2026-10-07).
- `ci/run.sh` à date fixe : `SOURCE_DATE_EPOCH` = date du dernier commit, ou valeur de
  l'environnement (2026-10-07, recommandation acceptée par l'utilisateur).

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `kal/arch/{armv7m,armv6m,host}/kal_arch.h` | `__va_list_from_arg` ; hôte : `__va_list_copy` = `va_copy`, branche `__x86_64__` |
| `scion/tests/host/mklepton_empreintes.sha256` | référence `-m32` des sorties de mklepton ; ne jamais régénérer |
| `ci/run.sh` | étape clang après le preset `host` ; `SOURCE_DATE_EPOCH` exporté (date fixe) |
| `$LEPTON_BUILD/etape-8/avant`, `apres` | artefacts CI avec heure de compilation (non comparables octet à octet) |
| `$LEPTON_BUILD/etape-8/sde/{avant,apres}.sha256` | `.bin` à date fixe : preuve d'identité |
| `$LEPTON_BUILD/etape-8/mass-compile-{avant,apres}.{md,csv}` | 426/444 des deux côtés, mêmes échecs |

## Occurrences restantes du `grep` de la tâche 4 (justifiées)
- `host.cmake:7`, `tests/host/CMakeLists.txt:48` : commentaires qui citent le retrait de `-m32`.
- `tools/migration/mklepton_oracle.sh` : exécute le binaire historique `mklepton_gnu` (ELF i386),
  oracle de l'étape 2 abandonné (libkernel.so absente) ; outil d'archive, non relancé.
- `tools/migration/dep_graph.py` : inventaire de l'étape 1 (`gcc -m32`), méthode d'archive.
- `tools/migration/kal_split.py:360` : texte de l'en-tête généré à l'étape 4, non relancé ;
  `test_kal_split.py` ne compare plus `kal/arch/host/kal_arch.h` (rejeu déjà ignoré).

## Écarts au plan et pièges découverts
- **`.bin` non reproductibles d'un build à l'autre** : `__DATE__`/`__TIME__` dans
  `core-segger/kernel.c`, `core-freertos/kernel.c` (et `core-static/kernel_static.c`) ; et
  l'éditeur de liens fusionne le littéral `"1"` de `ftpd.c:369` avec la fin d'une chaîne
  (`"USR1"` ou `__TIME__` quand l'heure finit par 1) : 2 octets de code changent aussi. Deux
  `ci/run.sh` successifs semblaient identiques parce que le second ne recompilait rien. Preuve
  retenue (plus forte que l'objdump prévu) : 14 presets construits avec `SOURCE_DATE_EPOCH=0`
  (GCC fige `__DATE__`/`__TIME__`), sources ARM de `master` puis `HEAD`, `.bin` identiques.
  Suite : `ci/run.sh` exporte `SOURCE_DATE_EPOCH` (date du commit par défaut). **Étape 9 :**
  `--target clean` des presets puis `SOURCE_DATE_EPOCH=0 ci/run.sh`, comparer au tableau ci-dessus
  (en incrémental, la date reste celle de la dernière compilation de `kernel.c`).
- **mass_compile 426/444, non 414/414** (valeur du fichier d'étape, relevée à la fin de
  l'étape 6) : 18 fichiers FreeRTOS reclassés actifs à l'étape 7 sont compilés avec le gabarit
  embOS (pas de profil FreeRTOS dans `mass_compile.py`) ; préexistant, inchangé ; en dette.
- `kal_arch.h` sont en UTF-8 (fichiers de la migration), `vfs.c` et `types.h` en ASCII :
  édition par script, encodage conservé.
- Avertissements 64 bits, consignés sans correction : gcc `time.c:512` (`-Wshift-count-overflow`) ;
  clang : 7 `-Wvarargs` (`kernel_io.c`), et `-Wpointer-sign`, `-Wparentheses`,
  `-Wtautological-constant-out-of-range-compare` (`dev_part.c`, `ufscore.c`, `systime.c`),
  `-Wlogical-not-parentheses` (`kernel_io.c:455`), `-Wheader-guard` (`mklepton.h:31`).
  `vfstypes.h:249` (`struct stat` dans une liste de paramètres) existait déjà en `-m32`.
- Hook `lepton_guard.py` : `$<TARGET_FILE:…>` et `$VAR/…` dans une commande sont lus comme des
  redirections vers le trunk : passer par un script du scratchpad.
- Appels `I_LINK` : aucun côté hôte (mklepton, tests, noyau statique) ; appels à trois arguments
  côté cible seulement (`core-*/kernel.c`, `kernel_elfloader.c`, `uip_core.c`), branche ARM
  inchangée.

## Non transmis volontairement
- Journaux : `$LEPTON_BUILD/ci_run_etape8_{avant1,avant2,apres}.log`,
  `$LEPTON_BUILD/etape-8/build-host-{gcc,clang}.log`, `etape-8/sde/<label>/<preset>.log`.
- Scripts du scratchpad (`etape8_source.py`, `cmp_bin.py`, `sde_bins.sh`) : rejouables depuis ce
  handoff et l'historique git.
