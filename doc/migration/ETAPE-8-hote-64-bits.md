# Étape 8 — Noyau statique hôte en 64 bits (retrait de `-m32`)

## Contexte

Le noyau statique hôte (bibliothèque de mklepton, preset `host`) est compilé en `-m32` depuis
l'étape 2 pour une seule raison : `_vfs_ioctl2` (`kernel/fs/vfs/vfs.c`, requête `I_LINK`) lit un
`va_list` reçu par argument variadique, ce que l'ABI x86_64 ne permet pas (`va_list` y est un
tableau). Le format UFS n'est pas en cause : il est identique x86_64 / i386 / arm-none-eabi
(`tests/host/ufs_format.c`, décision 2026-09-30).

Cette étape retire `-m32` **sur Debian**, hôte déjà validé : seule l'ABI de l'hôte change. Elle
est le préalable de l'hôte macOS (étape 9), qui ne peut ni lier ni exécuter un binaire i386.

Archétype : migration iso-fonctionnelle. Aucune sortie de mklepton et aucun binaire ARM ne change
d'un octet ; aucune refactorisation d'opportunité.

Essai hors dépôt du 2026-10-07 (copie de `master` `ae85f06`, Ubuntu 24.04, gcc 13.3 et clang 18.1,
x86_64) : les corrections des tâches 2 et 3 suffisent à construire en 64 bits, `ctest -L host`
5/5, sorties de mklepton identiques au `-m32` sur six mkconf. À reproduire ici, pas à tenir pour
acquis : ni Debian 13, ni gcc 14, ni le code objet ARM n'ont été vérifiés.

## Prérequis

- `master` à jour, `ci/run.sh` vert en l'état (avec `-m32`).
- Debian : paquet `clang` installé (second compilateur hôte, tâche 5).
- Décisions actées le 2026-10-07 : correction par macro d'architecture (l'alternative
  `va_list*` est reportée en dette) ; `-m32` retiré, paquets i386 compris.
- Branche `migration/etape-8`. Git local uniquement ; aucun push sans accord.

## Tâches

### 1. Références avant toute modification

- Empreintes de mklepton en `-m32` : construire le preset `host`, puis

  ```
  tests/host/mklepton_empreintes.sh --mklepton "$LEPTON_BUILD/host/mklepton" \
      --out "$LEPTON_CLONE/scion/tests/host/mklepton_empreintes.sha256"
  ```

  Versionner le fichier (commit séparé), `scion graft`. Il ne sera plus régénéré dans l'étape.
- Binaires ARM : lancer `ci/run.sh`, copier `$LEPTON_BUILD/ci/artefacts/` dans
  `$LEPTON_BUILD/etape-8/avant/` et y écrire les SHA-256 des `.bin` des 14 presets.
- Vérifier d'abord que ces `.bin` sont reproductibles : deux `ci/run.sh` de suite donnent les
  mêmes SHA-256. Sinon, noter la cause et comparer en tâche 6 le code objet désassemblé
  (`arm-none-eabi-objdump -d`) des objets touchés au lieu des `.bin`.

### 2. `va_list` reçu par argument variadique

- Ajouter la macro `__va_list_from_arg(dest, ap)` à côté de `__va_list_copy`, dans les trois
  `kernel/core/kal/arch/{armv7m,armv6m,host}/kal_arch.h` :

  | ISA | Développement |
  |---|---|
  | `armv7m`, `armv6m` | `dest = va_arg(ap, va_list)` — l'expression actuelle de `vfs.c`, inchangée |
  | `host`, x86_64 (`va_list` tableau : l'appelant a transmis un pointeur) | `va_copy(dest, *(va_list *)va_arg(ap, void *))` |
  | `host`, autre ABI | `dest = va_arg(ap, va_list)` |

- `kal/arch/host/kal_arch.h` : `__va_list_copy` devient `va_copy(dest, src)` (l'affectation
  actuelle ne compile pas quand `va_list` est un tableau). Ne pas toucher au `memcpy` des ISA ARM.
- `vfs.c`, dans `_vfs_ioctl2`, cas `I_LINK` : remplacer `__ap = va_arg(_ap, va_list);` par
  `__va_list_from_arg(__ap,_ap);`. Aucune autre ligne de `vfs.c`.
- La sélection par `__x86_64__` n'est permise que dans `kal/arch/host/` (CLAUDE.md §2).

### 3. `size_t`

`kernel/core/types.h` (« ugly patch for compatiblity IAR ARM7 ») définit `size_t` en
`unsigned int`. En LP64, gcc l'accepte en silence dans les unités où `types.h` précède
`<stddef.h>` (`size_t` de 4 octets) et prend 8 octets ailleurs ; clang le refuse.
Remplacer par `typedef __SIZE_TYPE__ size_t;`. Vérifier que `arm-none-eabi-gcc -dM -E` donne
`__SIZE_TYPE__` = `unsigned int` (valeur actuelle, donc sans effet sur la cible).

### 4. Retrait de `-m32`

- `cmake/isa/host.cmake` : retirer `add_compile_options(-m32)`, `add_link_options(-m32)`,
  l'argument de `lepton_freestanding(-m32)`, et réécrire le commentaire d'en-tête.
- `scripts/install-debian.sh` : retirer le bloc « Bibliothèques i386 » entier
  (`dpkg --add-architecture i386`, `libc6:i386`, `libexpat1:i386`, `gcc-multilib`,
  `libexpat1-dev:i386`) ; ajouter `clang` au bloc de base.
- Traiter chaque occurrence de `grep -rn -e "-m32" -e multilib -e ":i386" scion/cmake scion/tests
  scion/sys/root/src/kernel scripts ci tools/migration doc/BUILDING.md` : la corriger ou la
  justifier dans le handoff. `tools/migration/mass_compile.py` (profils host) en fait partie.
- Ne pas réécrire l'historique : `handoff/`, les décisions datées de `MIGRATION-STATUS.md` et
  les fichiers `ETAPE-0` à `ETAPE-7` restent tels quels.

### 5. Second compilateur hôte

- Construire le preset `host` avec clang dans un autre répertoire :
  `cmake --preset host -B "$LEPTON_BUILD/host-clang" -DCMAKE_C_COMPILER=clang`, build,
  `ctest --test-dir "$LEPTON_BUILD/host-clang" -L host`.
- Ajouter cette construction à `ci/run.sh`, après le preset `host`.

### 6. Test permanent et identité

- `tests/host/CMakeLists.txt` : test `host.mklepton_empreintes` (label `host`) lançant
  `mklepton_empreintes.sh --check` contre `tests/host/mklepton_empreintes.sha256`.
- Binaires ARM : `ci/run.sh`, puis comparer les SHA-256 des `.bin` des 14 presets à ceux de
  `$LEPTON_BUILD/etape-8/avant/`.

## Critères de validation

- [ ] `file "$LEPTON_BUILD/host/mklepton"` : « ELF 64-bit ».
- [ ] `ctest --preset host -L host` vert, `host.mklepton_empreintes` compris, avec gcc et avec
      clang.
- [ ] `git log --oneline -- scion/tests/host/mklepton_empreintes.sha256` : un seul commit, celui
      de la tâche 1 (la référence `-m32` n'a pas été régénérée).
- [ ] `.bin` des 14 presets ARM identiques avant et après (SHA-256), ou code objet désassemblé
      identique si les `.bin` ne sont pas reproductibles.
- [ ] `git diff master --stat -- scion/sys/root/src` : `vfs.c`, `types.h` et les trois
      `kal_arch.h`, rien d'autre.
- [ ] Le `grep` de la tâche 4 ne rend que des occurrences justifiées dans le handoff.
- [ ] `ci/run.sh` vert ; `tools/migration/mass_compile.sh` au même score qu'avant (414/414).

## Pièges connus

- **Appel `I_LINK` sans quatrième argument** (`_vfs_ioctl(desc, I_LINK, desc2)` :
  `core-segger/kernel.c`, `kernel_elfloader.c`, `uip_core.c`) : sur x86_64 la macro déréférence
  alors un argument absent. Aucun appel `I_LINK` dans le noyau statique, mklepton ni les tests
  hôte au 2026-10-07 (seulement `HDSETSZ`, `HDGETSZ`) ; le revérifier par `grep` avant de clore.
- **Sources en Latin-1** (`vfs.c`, `types.h`, `kal_arch.h`) : éditer par script
  (`encoding="latin-1"`), pas par l'outil d'édition, qui réécrit en UTF-8.
- **Fichier ajouté dans le clone** : `scion graft` avant de construire, sinon le trunk ne le
  voit pas.
- **Avertissements propres au 64 bits, à consigner sans corriger** : `kernel/core/time.c:512`
  (`-Wshift-count-overflow`, branche `LONG_MAX >> DBL_MANT_DIG`) ; avec clang, sept
  `-Wvarargs` dans `kernel/core/kernel_io.c` (`va_start` sur un paramètre promu).
- **`ssize_t` reste `int32_t`** dans `types.h` : ne pas l'aligner sur `size_t` (hors étape).
- **`kernel_stub.h`** : mklepton (glibc) et le noyau (freestanding) passent ensemble en LP64 ;
  `host.kernel_stub` est le test qui garde leur concordance, ne pas le contourner.
- Un `.bin` ARM qui change désigne `types.h` ou la branche ARM de la macro : revenir à
  l'expression d'origine avant de chercher ailleurs.

## À la fin de l'étape

`MIGRATION-STATUS.md` : ligne de l'étape 8 ; « Versions épinglées » (ligne `gcc-multilib`
remplacée, `clang` ajouté) ; dette : la ligne « `vfs.c` (I_LINK) … cause du `-m32` » devient
« corrigé par macro d'architecture (étape 8) ; reste : passer un `va_list*`, les appels à trois
arguments transmettant `NULL` ». `doc/BUILDING.md` : ligne « hôte x86 (`-m32`) » du tableau.
`handoff/etape-8.md`, qui donne en plus pour l'étape 9 : nombre de feuilles du trunk
(`scion rootstock-information`) et SHA-256 des `.bin` des 14 presets.
