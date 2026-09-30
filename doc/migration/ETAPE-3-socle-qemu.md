# Étape 3 — Noyau dynamique sur QEMU `mps2-an386` : UART, puis Ethernet

<!-- Source : sources/lepton-migration-guide-step-1.md (auteur, §2.1). -->

## Contexte

Premier binaire Lepton compilé par GCC et exécuté. La cible est une machine QEMU générique, pas un
STM32 : tout son matériel est émulé et connu, ce qui isole les défauts du portage (compilateur,
assembleur, édition de liens, KAL, appels système) de ceux d'une carte. Cette étape remplace la
simulation Linux (abandonnée) comme banc permanent : chaque étape suivante se revalide ici.
C'est le noyau **dynamique** (ordonnanceur actif), par opposition au noyau statique hôte de
l'étape 2. Elle porte **seulement la chaîne minimale** définie ci-dessous ; le portage de masse du
reste du périmètre est l'étape 4.

Chaîne minimale (guide §2.1, chemins relatifs à `sys/root/src/`) :

| Composant | Contenu |
|---|---|
| `kernel/core/core-segger` | enveloppe des primitives du noyau temps réel embOS |
| `kernel/core/kal` | couche d'abstraction propre au noyau temps réel et à la cible (aujourd'hui `kal.c`/`kal.h`) |
| `kernel/core` | base du noyau avec tous les appels système |
| `kernel/dev` | pilotes de la cible QEMU ; premier test : liaison série seule |
| `kernel/fs` | `vfs`, `rootfs`, `ufs` |
| `lib` | `libc`, `pthread` (hors de `kernel/`) |
| `sbin` | `lsh`, `ps`, `ls`, `uname` (hors de `kernel/`) |
| `bin` | pseudo-binaires de tests unitaires — contenu <À CONFIRMER> (proposition : niveau POSIX du banc KAL, T9-T11) |

## Découpage en deux sessions

- **3a — UART** : tâches 0 à 4 et 6, paliers 1 à 6 ; handoff `handoff/etape-3a.md`.
- **3b — Ethernet** : tâche 5, palier 7 ; commence par la relecture du handoff 3a.

## Prérequis

- Étapes 1 et 2 : liste de sources dérivée `mps2-an386`, presets, mklepton natif et image UFS
  (format vérifié compatible avec ARM 32 bits).
- `gcc-arm-none-eabi` (épinglé).
- `qemu-system-arm` ≥ 8.2 ; paquet embOS Cortex-M GCC (variante M4F) intégré.

## Machine cible (relevée sur QEMU 8.2, `info mtree`, `info network`)

| Ressource | Adresse | Usage |
|---|---|---|
| SSRAM1 4 Mo | `0x00000000` | code et vecteurs (`-kernel`) |
| SSRAM2/3 4 Mo | `0x20000000` | données, piles, tas |
| UART CMSDK 0 à 3, 4 | `0x40004000`–`0x40007000`, `0x40009000` | UART0 = console `lsh`, UART1 = 2ᵉ port ; chaque `-serial` alimente l'UART suivante |
| Timers CMSDK, SysTick | `0x40000000`, `0x40001000` | tick système (SysTick) |
| Ethernet LAN9118 | `0x40200000` | `-nic user,model=lan9118` |

Pas de gestion d'horloge ni de flash : aucun code spécifique à QEMU dans le noyau.

## Tâches

### 0. Toolchain croisée

- `cmake/toolchains/armv7m-gcc.cmake` (et `armv6m`) : `CMAKE_SYSTEM_NAME Generic`,
  `arm-none-eabi-gcc` pour C et ASM, `CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY`.
- `cmake/cpu/cortex-m4f.cmake` : `-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16`,
  à confirmer contre la variante de bibliothèque embOS.
- `cmake/flags-gnu.cmake` : `-ffunction-sections -fdata-sections`, `-Wl,--gc-sections`, `-Wl,-Map`,
  `-Wl,--print-memory-usage`, `-Og` en validation ; post-build `objcopy`, `size`.

### 1. Chaîne minimale compilable

- `compiler.h` (macros `__lepton_*`, branche GCC seulement ; IAR n'est plus supporté) et
  remplacement des intrinsics IAR par CMSIS-Core (vendored, épinglé), **limités aux fichiers de la
  chaîne minimale**, par un script amorcé ici (`tools/migration/transform_iar.py`, complété à
  l'étape 4), appliqué dans le clone, jamais dans le trunk.
- Macros de section critique à nom neutre (`__lepton_disable_irq`…) : prévues pour d'autres ISA.

### 2. Assembleur Lepton en syntaxe GNU

- Traduire uniquement l'assembleur propre à Lepton (le port GCC Segger fournit PendSV, SysTick et
  la commutation embOS) : démarrage Cortex-M générique, table des vecteurs, chaîne d'appel système
  (`__mk_syscall__` → SVC → `software_interrupt` → `_kernel_syscall_handler` → bascule vers la pile
  noyau et `_kernel_entry`).
- Correspondances : `SECTION`→`.section`, `PUBLIC`→`.global`, `EXTERN`→`.extern`, `DC32`→`.word`,
  `DS8`→`.space`, `EQU`→`.equ`, `THUMB`→`.thumb` + **`.thumb_func` avant chaque fonction**.
  Traduire les directives, jamais « améliorer » les instructions.
- Offsets de structures partagées C/asm générés (`asm-offsets`), jamais codés en dur ; ajustés
  d'après `embos-iar-vs-gcc.md`.
- Fichiers placés selon l'axe ISA (`doc/migration/ajout-coeur.md`) : ils serviront à toutes les
  cartes ARMv7-M.

### 3. Édition de liens et bibliothèque C

- `ld/common-cortexm.ld` (vecteurs `KEEP`, `.text`, `.data` `AT>`, `.bss`, `.noinit`, pile, tas,
  `.ARM.exidx`, `.init_array`, section du système de fichiers mklepton) + `ld/mem_qemu-mps2-an386.ld`.
- Glue newlib-nano (`_sbrk`, `_write`…) ; frontière documentée : newlib pour le noyau et le démarrage,
  API POSIX applicative = celle de Lepton. **Point d'arrêt** si la frontière est ambiguë.

### 4. BSP `qemu-mps2-an386`

- Intégration embOS (RTOSInit sur SysTick, variante M4F).
- Pilote UART CMSDK (nouveau : absent de `sys/root/src/kernel/dev/arch`), exposé comme périphérique
  standard de `lsh` ; 2ᵉ instance sur UART1.
- Test de fumée canonique `tests/smoke_lsh.py` : démarrage → prompt `lsh` → `uname -a`, champs
  vérifiés (`--expect-machine`), transport sur la série QEMU (`-serial stdio` ou `tcp`), code de
  retour CTest (label `smoke`).

### 5. Ethernet

- Pilote `dev_eth_lan9118` (nouveau) sous `dev/arch/all/eth/`, sur le modèle de
  `dev_eth_dm9000a` (contrôleur externe mappé en mémoire, même classe).
- Réseau QEMU : `-nic user,model=lan9118,hostfwd=tcp::<port>-:21` ; ping, puis `ftpd`
  (`sys/root/src/bin/net/ftpd`).

### 6. Banc KAL et non-régression

- Construire le banc `BANC-TEST-KAL-QEMU.md` sur cette machine : T0 puis T1 à T8.
- Écrire `ci/run.sh` (racine du clone, non greffé) : configure et construit les presets hôte et
  `qemu-mps2-an386-embos`, puis `ctest -L host`, `-L smoke`, `-L kal` ; code de retour non nul au
  premier échec. C'est la commande de non-régression exigée avant chaque commit à partir d'ici
  (ORCHESTRATION §5) ; l'étape 6 l'étend et la branche sur la CI.

## Paliers de validation (dans l'ordre, arrêt au premier échec)

1. Reset → `main` (MSP et PC initiaux lus dans les vecteurs).
2. Mémoire : `.data` copiée, `.bss` à zéro, `.noinit` intacte.
3. Warmup du noyau (`_kernel_warmup_*` : rootfs, montage du UFS de `dev_cpufs` produit par mklepton, pilotes, objets noyau).
4. Premier appel système tracé pas à pas (`qemu -s -S` + `gdb-multiarch`).
5. Multitâche : pthreads, préemption, signaux.
6. Test de fumée canonique vert sur UART0 (`lsh`, `uname -a`), puis `ls` et `ps` ; second port UART1
   fonctionnel.
7. Réseau : ping depuis l'hôte, session `ftpd`.

## Critères de validation

- [ ] Paliers 1 à 7 verts ; `ci/run.sh` vert.
- [ ] Banc KAL : T0 à T8 verts (M4F × embOS).
- [ ] Aucune adresse de `mps2-an386` hors de `cmake/boards/`, `ld/mem_*` et du BSP.
- [ ] Journal des paliers : `doc/migration/validation-qemu-mps2-an386.md`.

## Pièges connus

- HardFault au premier appel par vecteur : `.thumb_func` oublié (adresse paire, vérifier avec `nm`).
- FPU : activer CPACR dans le démarrage (M4F), sinon faute à la première instruction flottante.
- `--gc-sections` sans `KEEP` sur les vecteurs : le binaire se lie mais ne démarre pas.
- QEMU est permissif sur le timing et certaines fautes : un vert QEMU ne dispense pas de l'étape 5.

## À la fin de l'étape

`MIGRATION-STATUS.md` : socle QEMU validé, paliers, matrice du banc ; `handoff/etape-3.md`.
