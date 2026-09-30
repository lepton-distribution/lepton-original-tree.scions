# Noyau statique hôte (bibliothèque de mklepton) — inventaire étape 1, tâche 5

Sources : `sys/root/prj/scons/arch/synthetic/x86_static/SConstruct`, guide de l'auteur
(`sources/lepton-migration-guide-step-1.md` §1.1), arbre composé `trunk/` au 2026-09-30.
Intermédiaires rejouables : `$LEPTON_BUILD/etape-1/mklepton/` (`syntax.sh`, `objs.sh`,
`classify.py`, `probe.sh`, stubs `stub/`). Aucun fichier du trunk modifié.

## 1. Construction historique (`x86_static/SConstruct`)

- Produit `libkernel.so` (**bibliothèque partagée** i386, malgré le nom « static ») installée dans
  `$HOME/tauon/sys/root/prj/scons/arch/synthetic/x86_static/bin/` ; `base_dir = $HOME/tauon/sys/root/`
  codé en dur.
- Drapeaux : `-g -O0 -std=c99 -ffunction-sections -fdata-sections -fno-builtin -DCPU_GNU32
  -DUSE_KERNEL_STATIC`, include eCos `lib/arch/synthetic/x86/install/include` (absent), édition
  `-nostartfiles -nostdlib`. Compilateur hôte `gcc` (i386 à l'époque ; `i386-elf-gcc` commenté).
- `prj/scons/common/` (`SConscript`, `module/tauon.py`) ne concerne pas `x86_static` (projets ARM/cortexm).
- Liste exhaustive fichier par fichier : `noyau-statique-sources.csv` (78 entrées).

| Groupe | Fichiers | Présents | Absents | Commentaire |
|---|---|---|---|---|
| `kernel/core/core-ecos/` | 13 | 0 | 13 | répertoire absent ; homonymes dans `core-segger/` et `core-freertos/` |
| `kernel/core/*.c` | 19 | 18 | 1 | `devio.c` absent (HYPOTHÈSE À VALIDER : remplacé par `kernel_io.c`) |
| `kernel/core/arch/synthetic/x86_static/` | 2 (+1) | 0 | 3 | `bin_mkconf.c`, `dev_mkconf.c` et `kernel_mkconf.h` (inclus par `kernelconf.h:99`) absents ; modèles générés dans `kernel/core/arch/win32/` |
| `kernel/fs/vfs`, `rootfs`, `ufs` | 12 | 12 | 0 | |
| `kernel/fs/kofs`, `fat` | 7 | 7 | 0 | hors périmètre du guide |
| `lib/libc`, `lib/pthread`, `sbin` | 20 | 20 | 0 | hors périmètre du guide |
| `kernel/dev/arch/gnu32/` | 4 | 4 | 0 | code gelé (simulation Linux) ; fournit `/dev/hd/hdc` (`.fsflash.o`), `/dev/rtc0`, appels système Linux `int $0x80` (i386 seul) |

Constat hors arbre (non exploité) : des copies historiques contenant `core-ecos/`,
`core/arch/synthetic/x86_static/` et `libkernel.so` (sha256 `aae48adb…`, 904 971 octets, identiques
entre copies) existent sous `/mnt/hgfs/entreprises/lepton/…` (dossier partagé de l'hôte). Leur
lecture a été bloquée par la politique de l'agent : **décision utilisateur** (voir `mklepton.md` §5).

## 2. Périmètre cible (guide §1.1) et compilation d'essai sur l'hôte

Périmètre : `kernel/core/*.c` (hors sous-répertoires), `kernel/dev/dev_{cpufs,head,null,proc,tty}`,
`kernel/fs/{vfs,rootfs,ufs}` — 45 fichiers. Aucun backend de micro-noyau n'est inclus.

Essai `gcc -fsyntax-only` / `-c` (hôte x86_64, `-DCPU_GNU32 -DUSE_KERNEL_STATIC`,
stub `kernel_mkconf.h` repris de `arch/win32`) :

1. **Avec la glibc** : 44/45 fichiers en erreur — les types Lepton (`kernel/core/types.h`,
   `etypes.h` : `pid_t`=int16, `__ino_t`, `__time_t`=uint32, `dev_t`, `off_t`, `sigset_t`,
   `pthread_*`, `struct timespec`…) entrent en conflit avec `<sys/types.h>`. Le noyau statique doit
   être compilé **freestanding** (`-ffreestanding -nostdinc`), avec des en-têtes libc minimaux
   (Lepton `lib/libc` ou newlib) — c'est ce que faisait l'include eCos. mklepton, lui, ne voit que
   `kernel_stub.h` (voir `mklepton.md`).
2. **Freestanding + stubs** (`string.h`, `stdlib.h`, `ctype.h` minimaux) : 39/45 OK. Échecs réels :

| Fichier:ligne | Erreur | Nature |
|---|---|---|
| `core/kal.h:535` (via `fs/vfs/vfs.c:917`) | `__va_list_copy` = affectation de `va_list` | spécifique i386 ; x86_64/ARM exigent `va_copy` |
| `fs/vfs/vfs.c:1526,1534,1652,1664,1679` | `readdir` : pointeur incompatible | erreur par défaut en GCC 14 |
| `core/kernel_io.c:145` | `__wait_io_int2` non défini sous `USE_KERNEL_STATIC` | dépendance ordonnanceur |
| `core/system.c:267` | `_SYSCALL_PTHREAD_KILL` non défini | appel système |
| `core/sysctl.c:78` | `__KERNEL_CPU_DEVICE_NAME` non défini | config cpu `gnu32` |
| `core/net.c:31` | `kernel/errno.h` introuvable | chemin d'include cassé |
| `core/time.c` | `<limits.h>` (include_next) | en-tête libc à fournir |

## 3. Dépendances à couper (ordonnanceur, KAL, appels système)

Symboles non résolus du périmètre (nm sur les 39 objets compilés), hors libc et hors symboles
définis dans le périmètre ; tous sont définis aujourd'hui dans `core/core-segger/` (et
`core-freertos/`), qui dépendent du micro-noyau :

| Symbole | Utilisé par | Défini dans | Classe |
|---|---|---|---|
| `kernel_pthread_mutex_{init,lock,unlock,destroy}` | `core/kernel_mqueue.c`, `posix_mqueue.c`, `pipe.c`, `select.c:73`, `system.c:286-288`, `fs/vfs/vfs.c` | `core-segger/kernel_pthread_mutex.c` (déjà court-circuité si `__kernel_is_in_static_mode()`, l. 158/178/199) | ordonnanceur |
| `kernel_sem_{init,post,wait,trywait,timedwait}` | `kernel_mqueue.c`, `posix_mqueue.c`, `select.c`, `vfs.c:1848-1849` | `core-segger/kernel_sem.c` | ordonnanceur |
| `kernel_pthread_self` | `kernel_io.c:89,256,463`, `select.c:103`, `system.c:253`, `timer.c:119` (+ `dirent/fcntl/stat/statvfs/truncate/wait.c` via `__mk_syscall`) | `core-segger/kernel_pthread.c` | ordonnanceur |
| `kernel_timer_settime` | `timer.c:120` | `core-segger/kernel_timer.c` | ordonnanceur |
| `__mk_syscall` (macro → `kernel_pthread_self` + SVC) | `dirent.c` (7), `stat.c` (5), `statvfs.c` (3), `system.c` (~20), `time.c:79,98`, `timer.c:61,76`, `truncate.c:69,88`, `wait.c:61,84`, `fcntl.c:98` | `core/syscall.h` | appel système — **fichiers entiers à exclure** (API POSIX utilisateur) |
| `__wait_io_int*`, `__fire_io_int`, `__syscall_lock/unlock`, `__atomic_in/out` | `kernel_io.c:145,336,352`, `select.c:309,313`, `pipe.c:348-448`, `posix_mqueue.c:311-317`, `net.c:402,414`, `vfskernel.c`, `malloc.c` | macros `kal.h`/`interrupt.h` (branche `USE_KERNEL_STATIC` partielle, `interrupt.h:799`) | KAL |
| `_syscall_owner_pid`, `_syscall_owner_pthread_ptr` | `fs/vfs/vfscore.c`, `vfs.c`, `rootfs.c`, `ufs.c`, `ufscore.c`, `ufsx.c` | `core-segger/kernel.c` | noyau (à fournir par un backend statique) |
| `_kernel_in_static_mode`, `__g_kernel_static_errno` | idem fs | `core-segger/kernel.c:86,140` | noyau — **mécanisme statique existant** (`kernel.h:179-197,349-355`) |
| `process_lst`, `g_pthread_lst`, `kernel_mutex`, `__fds_size`, `__shl_fds_bits`, `_get_fd`, `_put_fd`, `_unset_cloexec` | `flock.c:210`, `pipe.c:261`, `select.c`, `dev_proc.c`, `vfscore.c`, `vfskernel.c` | `core-segger/process.c`, `kernel.c`, `kernel_pthread.c` | noyau |
| `_syscall_kill` | `pipe.c:261` | `core-segger/syscall.c` | appel système |
| `__g_kernel_cpu`, `__g_kernel_desc_cpu` | `cpu.c` | `core-segger/kernel.c` | noyau |
| `_kernel_warmup_{rootfs,dev,rtc,mount}` (requis par mklepton) | — | `core-segger/kernel.c` | amorçage à extraire |
| `bin_lst`, `bin_lst_size`, `dev_lst`/`pdev_lst`/`max_dev`, `filecpu_memory(_size)` | `bin.c`, `vfs.c`, `dev_cpufs.c` | générés par mklepton (`bin_mkconf.c`, `dev_mkconf.c`, `dev_dskimg.c`) | œuf et poule : le noyau statique de mklepton a besoin de ses propres fichiers générés (fournis à la main dans `x86_static/`, absents) |
| `kofs_op` | `vfscore.c` | `fs/kofs/kofs.c` | à désactiver (`__KERNEL_VFS_SUPPORT_KOFS`) |
| `tty_font_info_vga_8x{8,16}` | `dev_tty.c` | `dev_tty/tty_font-8x*.c` | à ajouter à la liste |
| `libc_lib_entrypoint` | `core/lib.c` | introuvable | `lib.c` à exclure |

Conséquence : le noyau statique = périmètre du guide **moins** les fichiers d'API système
(`dirent.c`, `stat.c`, `statvfs.c`, `system.c`, `time.c`, `timer.c`, `truncate.c`, `wait.c`,
`fcntl.c`, `kernel_mqueue.c`, `posix_mqueue.c`, `select.c`, `net.c`, `lib.c`) **plus** un petit
backend « statique » (successeur de `core-ecos/` + `x86_static/`) fournissant : `_kernel_in_static_mode=1`,
`_syscall_owner_*`, `process_lst`/`g_pthread_lst` (un seul processus), stubs mutex/sem/self,
`_kernel_warmup_*`, `__wrpr_kernel_dev_gettime`, `xtime`, et les tables `dev_lst`/`bin_lst`.
Liste finale à arrêter à l'étape 2 avec le graphe de dépendances (tâche 6).

**Point ouvert — second pilote logiciel.** Le guide cite `dev_null` deux fois. mklepton exige en
plus un disque hôte `/dev/hd/hdc` (écrit dans `.fsflash.o`) et une horloge `/dev/rtc0` : fournis par
les pilotes `gnu32` gelés (`dev_linux_fileflash`, `dev_linux_rtc`, appels `int $0x80` i386). Les
candidats au second pilote sont donc un **pilote disque sur fichier hôte** (réécriture portable de
`dev_linux_fileflash`) ou `dev_pipe` (présent dans `dev_mkconf.c` win32). HYPOTHÈSE À VALIDER par
l'utilisateur. `dev_head`, `dev_proc`, `dev_tty` ne sont pas nécessaires à mklepton (non ouverts par
`mklepton.c`).

## 4. Structures écrites dans l'image UFS

Pilote par défaut : `__KERNEL_SUPPORT_UFS_DRIVER = 1_5` (`kernelconf.h:650`) ; le nœud 1.5 est le
`ufs_block_node_1_4_t` (`ufscore.h`). Disposition (`ufscore.c:_ufs_makefs`, `ufsdriver_1_5.c`) :

| Zone | Contenu | Écriture |
|---|---|---|
| en-tête superbloc | `"ufs 1.5"` (7 o, sans NUL), `blk_size` u16, `superblk_size`, `alloc_blk_size`, `alloc_node_size`, `nodeblk_size`, `datablk_size` (u32 chacun) = 29 o | **champ par champ** (pas de `sizeof(superblk_t)`) |
| superbloc | bitmaps allocation blocs (`max_blk/8+1`) puis nœuds (`max_node/8+1`) | octets |
| table des nœuds | `max_node × sizeof(ufs_block_node_1_4_t)` | struct brute |
| blocs de données | `max_blk × blk_size` : données, blocs d'indirection (`ufs_blocknb_t`=int16), entrées de répertoire `ufs_block_dir_t` | struct brute |
| fichiers pseudo-binaires | `exec_file_t` (8 o) écrit par mklepton | struct brute (déclaration **dupliquée** dans `kernel_stub.h`) |

Tailles mesurées (`probe.sh` : `nm -S` sur objets compilés, freestanding) — x86_64, i386
(`-m32`), « ARM approx. » (`-m32 -malign-double -fshort-enums -funsigned-char`, arm-none-eabi-gcc
absent le 2026-09-30) :

| Type (sur disque) | x86_64 | i386 | ARM approx. | Détail |
|---|---|---|---|---|
| `ufs_block_node_1_4_t` | 24 / al. 4 | 24 | 24 | `attr`@0 u16, `size`@4 i32, `ino_mod`@8 (u16, bitfields 9+4+3), `cmtime`@12 **time_t Lepton = u32**, `blk[1]`@16, `blk_smpl`@18, `blk_dbl`@20 (i16), pad 2 |
| `ufs_block_dir_t` (ufs) | 16 | 16 | 16 | `inode` i16 + `name[14]` ; `ufsx` : 32 (`name[30]`) |
| `ufs_block_indirect_t` | 256 | 256 | 256 | = `UFS_BLOCK_SIZE_MAX` (256 ici, 64 par défaut) — tampon mémoire |
| `exec_file_t` | 8 | 8 | 8 | char, u8, u16, i16, i16 |
| `ino_mod_t` | 2 | 2 | 2 | bitfields u16, petit-boutiste partout |

Types divergents (mémoire uniquement, non écrits tels quels) :

| Type | x86_64 | i386 | ARM approx. | Risque |
|---|---|---|---|---|
| `superblk_t` | 48 (al. 8) | 40 | 40 | aucun : écrit champ par champ ; contient `char* psuperblk` |
| `struct vfs_formatopt_t` | 24 (`dev_sz` long @16) | 16 (@12) | 16 | ABI mklepton ↔ noyau : `kernel_stub.h` doit rester cohérent |
| `fstype` (enum) | 4 | 4 | **1** | `-fshort-enums` : défaut arm-none-eabi (AAPCS non Linux) — HYPOTHÈSE À VALIDER avec arm-none-eabi-gcc ; non écrit sur disque |
| `long`, pointeurs, `size_t` | 8 | 4 | 4 | non écrits sur disque |

Conclusions pour l'étape 2 :
- Avec GCC, le format disque est **identique** hôte x86_64 / i386 / ARM (petit-boutiste, aucun champ
  long/pointeur/size_t, `time_t` Lepton 32 bits). À confirmer par `probe.sh` dès que
  `arm-none-eabi-gcc` est installé (le script l'utilise automatiquement) et par comparaison d'images.
- **Divergence IAR/MSVC vs GCC** : hors GCC, `kernelconf.h:193` supprime `__attribute__` et
  `ufscore.h`/`ufsdriver_1_*.h` activent `#pragma pack(1)` → nœud 1.4 compacté à 18 o (calcul :
  2+4+2+4+2+2+2). Les images produites par `mklepton.exe` (MSVC) visent les cibles IAR et sont
  **incompatibles** avec un noyau GCC ; l'oracle doit être `mklepton_gnu`.
- `cmtime` provient de l'horloge hôte (`_sys_gettimeofday`, `dev_linux_rtc`) : champ non
  déterministe, à masquer dans les comparaisons (offset 12, 4 o par nœud).
- Défaut latent : `etypes.h:58-64` définit `int64_t` = `long` quand
  `__KERNEL_COMPILER_SUPPORT_TYPE`=32 (GCC hors M0) → 32 bits en ILP32 ; et `int32_t`=`int`
  entre en conflit avec `<stdint.h>` de newlib (`long`). Garde `__KERNEL_COMPILER_STDINT_INCLUDED__`
  non posée pour GCC (`kernelconf.h:212-219`). À traiter à l'étape 4 (`compiler.h`).
- `kernel_stub.h` (mklepton) diverge déjà du noyau : `_vfs_statvfs_st` en `unsigned short` contre
  `fsblkcnt_t`=u32 (`types.h`) — affichage seulement, mais preuve que le stub n'est plus synchronisé.
