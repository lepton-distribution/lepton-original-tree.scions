# Transformation des IAR-ismes — rapport

Généré par `tools/migration/transform_iar.py` — ne pas éditer à la main.  
Commande : `transform_iar.py --perimetre-actif doc/migration/perimetre.csv --tiers-from-audit doc/migration/audit-iar.csv --report doc/migration/residuel-etape4.md -q`

Résumé : 23 fichier(s) modifié(s), 92 occurrence(s) automatique(s), 26 résiduelle(s).

## Résiduels (26)

| Fichier | Ligne | Règle | Motif | Détail |
|---|---:|---|---|---|
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2 | garde-iar-arm | branche IAR avec _Pragma/#pragma (placement, étape 5) | `defined(__IAR_SYSTEMS_ICC__) && _DLIB_INCLUDE_DLMALLOC_ALTERNATIVE` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 4 | garde-iar-arm | branche IAR avec _Pragma/#pragma (placement, étape 5) | `defined(__IAR_SYSTEMS_ICC__)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3 | header-iar | en-tête IAR yvals.h : équivalent CMSIS/newlib ou branche gelée à traiter à la main | `#include <yvals.h>` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 72 | header-iar | en-tête IAR yvals.h : équivalent CMSIS/newlib ou branche gelée à traiter à la main | `#include <yvals.h>` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 73 | header-iar | en-tête IAR ysizet.h : équivalent CMSIS/newlib ou branche gelée à traiter à la main | `#include <ysizet.h>` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 42 | symbole-iar | symbole IAR __iar_Locksyslock (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define PREACTION(M)  (__iar_Locksyslock(_LOCK_MALLOC), 0)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 43 | symbole-iar | symbole IAR __iar_Unlocksyslock (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define POSTACTION(M) __iar_Unlocksyslock(_LOCK_MALLOC)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 51 | symbole-iar | symbole IAR __iar_dlcalloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlcalloc               __iar_dlcalloc` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 52 | symbole-iar | symbole IAR __iar_dlfree (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlfree                 __iar_dlfree` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 53 | symbole-iar | symbole IAR __iar_dlmalloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmalloc               __iar_dlmalloc` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 54 | symbole-iar | symbole IAR __iar_dlmemalign (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmemalign             __iar_dlmemalign` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 55 | symbole-iar | symbole IAR __iar_dlrealloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlrealloc              __iar_dlrealloc` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 56 | symbole-iar | symbole IAR __iar_dlvalloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlvalloc               __iar_dlvalloc` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 57 | symbole-iar | symbole IAR __iar_dlpvalloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlpvalloc              __iar_dlpvalloc` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 58 | symbole-iar | symbole IAR __iar_dlmallinfo (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmallinfo             __iar_dlmallinfo` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 59 | symbole-iar | symbole IAR __iar_dlmallopt (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmallopt              __iar_dlmallopt` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 60 | symbole-iar | symbole IAR __iar_dlmalloc_trim (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmalloc_trim          __iar_dlmalloc_trim` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 61 | symbole-iar | symbole IAR __iar_dlmalloc_stats (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmalloc_stats         __iar_dlmalloc_stats` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 62 | symbole-iar | symbole IAR __iar_dlmalloc_usable_size (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmalloc_usable_size   __iar_dlmalloc_usable_size` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 63 | symbole-iar | symbole IAR __iar_dlmalloc_footprint (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmalloc_footprint     __iar_dlmalloc_footprint` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 64 | symbole-iar | symbole IAR __iar_dlmalloc_max_footprint (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmalloc_max_footprint __iar_dlmalloc_max_footprint` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 65 | symbole-iar | symbole IAR __iar_dlindependent_calloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlindependent_calloc   __iar_dlindependent_calloc` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 66 | symbole-iar | symbole IAR __iar_dlindependent_comalloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlindependent_comalloc __iar_dlindependent_comalloc` |
| `sys/user/tauon-basic/src/bin/free/free_main.c` | 59 | symbole-iar | symbole IAR __iar_dlmallinfo (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `extern struct mallinfo __iar_dlmallinfo();` |
| `sys/user/tauon-basic/src/bin/free/free_main.c` | 81 | symbole-iar | symbole IAR __iar_dlmallinfo (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `info = __iar_dlmallinfo();` |
| `sys/user/tauon-basic/src/bin/sdramtest/sdramtest_main.c` | 20 | pragma-iar | section EXT_RAM absente des scripts ld/*.ld (carte, étape 5) | `#define EXT_RAM_REGION      _Pragma("location = \"EXT_RAM\"")` |

## Automatiques (92)

| Fichier | Ligne | Règle | Motif | Détail |
|---|---:|---|---|---|
| `sys/root/src/bin/test2.c` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/bin/test2.c` |
| `sys/root/src/bin/test2.c` | 1665 | garde-cible-gelee | branche cible gelée retirée | `defined (__IAR_SYSTEMS_ICC)` |
| `sys/root/src/kernel/fs/fat/fatcore.h` | 236 | garde-cible-gelee | condition simplifiée | `defined(CPU_GNU32) \|\| defined(CPU_ARM9) → defined(CPU_GNU32)` |
| `sys/root/src/kernel/fs/ufs/ufs.c` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/fs/ufs/ufs.c` |
| `sys/root/src/kernel/fs/ufs/ufs.c` | 194 | garde-cible-gelee | branche cible gelée retirée | `defined(CPU_ARM7) \|\| defined(CPU_WIN32)` |
| `sys/root/src/kernel/fs/ufs/ufs.c` | 228 | garde-cible-gelee | #else toujours pris | `` |
| `sys/root/src/kernel/fs/ufs/ufscore.c` | 43 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/root/src/kernel/fs/ufs/ufscore.h` | 1 | garde-compilateur | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/fs/ufs/ufscore.h` |
| `sys/root/src/kernel/fs/ufs/ufscore.h` | 43 | garde-compilateur | branche compilateur retirée | `(__tauon_compiler__!=__compiler_gnuc__)` |
| `sys/root/src/kernel/fs/ufs/ufscore.h` | 116 | garde-compilateur | branche compilateur retirée | `(__tauon_compiler__!=__compiler_gnuc__)` |
| `sys/root/src/kernel/fs/ufs/ufsdriver_1_3.h` | 1 | garde-compilateur | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/fs/ufs/ufsdriver_1_3.h` |
| `sys/root/src/kernel/fs/ufs/ufsdriver_1_3.h` | 43 | garde-compilateur | branche compilateur retirée | `(__tauon_compiler__!=__compiler_gnuc__)` |
| `sys/root/src/kernel/fs/ufs/ufsdriver_1_3.h` | 68 | garde-compilateur | branche compilateur retirée | `(__tauon_compiler__!=__compiler_gnuc__)` |
| `sys/root/src/kernel/fs/ufs/ufsdriver_1_4.h` | 1 | garde-compilateur | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/fs/ufs/ufsdriver_1_4.h` |
| `sys/root/src/kernel/fs/ufs/ufsdriver_1_4.h` | 43 | garde-compilateur | branche compilateur retirée | `(__tauon_compiler__!=__compiler_gnuc__)` |
| `sys/root/src/kernel/fs/ufs/ufsdriver_1_4.h` | 67 | garde-compilateur | branche compilateur retirée | `(__tauon_compiler__!=__compiler_gnuc__)` |
| `sys/root/src/kernel/fs/ufs/ufsx.c` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/fs/ufs/ufsx.c` |
| `sys/root/src/kernel/fs/ufs/ufsx.c` | 224 | garde-cible-gelee | branche cible gelée retirée | `defined(CPU_ARM7) \|\| defined(CPU_WIN32)` |
| `sys/root/src/kernel/fs/ufs/ufsx.c` | 255 | garde-cible-gelee | #else toujours pris | `` |
| `sys/root/src/kernel/fs/vfs/vfs.c` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/fs/vfs/vfs.c` |
| `sys/root/src/kernel/fs/vfs/vfs.c` | 1879 | garde-cible-gelee | branche cible gelée retirée | `defined(__KERNEL_UCORE_ECOS)` |
| `sys/root/src/kernel/fs/vfs/vfs.c` | 1889 | garde-cible-gelee | branche cible gelée retirée | `defined(__KERNEL_UCORE_ECOS)` |
| `sys/root/src/kernel/fs/vfs/vfs.c` | 1 | garde-compilateur | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/fs/vfs/vfs.c` |
| `sys/root/src/kernel/fs/vfs/vfs.c` | 907 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/root/src/kernel/fs/vfs/vfs.c` | 909 | garde-compilateur | branche morte (précédente toujours vraie) | `` |
| `sys/root/src/kernel/fs/vfs/vfs.h` | 34 | garde-compilateur | garde toujours vraie levée | `(__tauon_compiler__==__compiler_keil_arm__) \|\| (__tauon_compiler__==__compiler_gnuc__)` |
| `sys/root/src/kernel/fs/vfs/vfskernel.c` | 42 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/root/src/lib/libc/ctype/ctype.c` | 42 | garde-cible-gelee | garde toujours vraie levée | `!defined(__KERNEL_UCORE_ECOS)` |
| `sys/root/src/lib/libc/ctype/ctype.h` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/lib/libc/ctype/ctype.h` |
| `sys/root/src/lib/libc/ctype/ctype.h` | 37 | garde-cible-gelee | branche cible gelée retirée | `defined(CPU_WIN32)` |
| `sys/root/src/lib/libc/ctype/ctype.h` | 42 | garde-cible-gelee | garde toujours vraie levée | `!defined(__KERNEL_UCORE_ECOS)` |
| `sys/root/src/lib/libc/stdio/printf.c` | 1 | garde-compilateur | copie à l'identique (code gelé) | `legacy/sys/root/src/lib/libc/stdio/printf.c` |
| `sys/root/src/lib/libc/stdio/printf.c` | 1181 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/root/src/lib/libc/stdio/printf.c` | 1186 | garde-compilateur | branche morte (précédente toujours vraie) | `` |
| `sys/root/src/lib/libc/stdio/printf.c` | 1216 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/root/src/lib/libc/stdio/printf.c` | 1221 | garde-compilateur | branche morte (précédente toujours vraie) | `` |
| `sys/root/src/lib/libc/stdio/stdio.c` | 1 | garde-compilateur | copie à l'identique (code gelé) | `legacy/sys/root/src/lib/libc/stdio/stdio.c` |
| `sys/root/src/lib/libc/stdio/stdio.c` | 68 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/root/src/lib/libc/stdio/stdio.c` | 70 | garde-compilateur | branche morte (précédente toujours vraie) | `` |
| `sys/root/src/lib/libc/stdio/stdio.c` | 78 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/root/src/lib/libc/stdio/stdio.c` | 80 | garde-compilateur | branche morte (précédente toujours vraie) | `` |
| `sys/root/src/lib/libc/stdio/stdio.c` | 94 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/root/src/lib/libc/stdio/stdio.c` | 96 | garde-compilateur | branche morte (précédente toujours vraie) | `` |
| `sys/root/src/lib/libc/stdio/stdio.h` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/lib/libc/stdio/stdio.h` |
| `sys/root/src/lib/libc/stdio/stdio.h` | 106 | garde-cible-gelee | condition simplifiée | `defined(CPU_WIN32) \|\| defined(CPU_GNU32) → defined(CPU_GNU32)` |
| `sys/root/src/lib/libc/stdio/stdio.h` | 108 | garde-cible-gelee | branche cible gelée retirée | `defined(CPU_ARM9)` |
| `sys/root/src/lib/libc/stdio/stdio.h` | 110 | garde-cible-gelee | branche cible gelée retirée | `defined(CPU_ARM7) \|\| defined(CPU_M16C62)` |
| `sys/root/src/lib/libc/unistd/io.c` | 1 | garde-compilateur | copie à l'identique (code gelé) | `legacy/sys/root/src/lib/libc/unistd/io.c` |
| `sys/root/src/lib/libc/unistd/io.c` | 120 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/root/src/lib/libc/unistd/io.c` | 123 | garde-compilateur | branche morte (précédente toujours vraie) | `` |
| `sys/root/src/lib/libc/unistd/io.c` | 178 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/root/src/lib/libc/unistd/io.c` | 181 | garde-compilateur | branche morte (précédente toujours vraie) | `` |
| `sys/root/src/lib/libc/unistd/io.c` | 208 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/root/src/lib/libc/unistd/io.c` | 211 | garde-compilateur | branche morte (précédente toujours vraie) | `` |
| `sys/root/src/lib/libc/unistd/io.c` | 411 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/root/src/lib/libc/unistd/io.c` | 414 | garde-compilateur | branche morte (précédente toujours vraie) | `` |
| `sys/root/src/lib/libc/unistd/io.c` | 462 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/root/src/lib/libc/unistd/io.c` | 465 | garde-compilateur | branche morte (précédente toujours vraie) | `` |
| `sys/root/src/lib/librt/mq.c` | 1 | garde-compilateur | copie à l'identique (code gelé) | `legacy/sys/root/src/lib/librt/mq.c` |
| `sys/root/src/lib/librt/mq.c` | 89 | garde-compilateur | branche compilateur retirée | `!defined(__GNUC__)` |
| `sys/root/src/lib/librt/mq.c` | 91 | garde-compilateur | #else toujours pris | `` |
| `sys/root/src/sbin/ps.c` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/sbin/ps.c` |
| `sys/root/src/sbin/ps.c` | 79 | garde-cible-gelee | branche cible gelée retirée | `defined(__KERNEL_UCORE_ECOS)` |
| `sys/root/src/sbin/ps.c` | 123 | garde-cible-gelee | branche cible gelée retirée | `defined(__KERNEL_UCORE_ECOS)` |
| `sys/root/src/sbin/ps.c` | 133 | garde-cible-gelee | #else toujours pris | `` |
| `sys/root/src/sbin/stty.c` | 268 | prototype-static | prototype rendu static (output) | `case CS5: output("cs5"); break;` |
| `sys/root/src/sbin/stty.c` | 269 | prototype-static | prototype rendu static (output) | `case CS6: output("cs6"); break;` |
| `sys/root/src/sbin/stty.c` | 270 | prototype-static | prototype rendu static (output) | `case CS7: output("cs7"); break;` |
| `sys/root/src/sbin/stty.c` | 271 | prototype-static | prototype rendu static (output) | `case CS8: output("cs8"); break;` |
| `sys/root/src/sbin/stty.c` | 272 | prototype-static | prototype rendu static (output) | `default: output("cs??"); break;` |
| `sys/root/src/sbin/xmodem.c` | 103 | garde-cible-gelee | branche cible gelée retirée | `defined(CPU_M16C62)` |
| `sys/root/src/sbin/xmodem.c` | 104 | garde-cible-gelee | condition simplifiée | `defined(CPU_ARM7) \|\| defined(CPU_ARM9) \|\| defined(CPU_WIN32) \|\| defined(__UNIX__) → defined(__UNIX__)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1438 | garde-iar-arm | condition simplifiée | `!defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__) &&     !defined(__IAR_SYSTEMS_ICC__) → !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3124 | garde-iar-arm | branche IAR ARM retirée | `__IAR_SYSTEMS_ICC__` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3500 | garde-iar-arm | branche IAR ARM retirée | `defined(__IAR_SYSTEMS_ICC__) && !_DLIB_FULL_LOCALE_SUPPORT` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3504 | garde-iar-arm | #else toujours pris | `` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 4087 | garde-iar-arm | branche IAR ARM retirée | `defined(__IAR_SYSTEMS_ICC__)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1 | garde-compilateur | copie à l'identique (code gelé) | `legacy/sys/user/tauon-basic/src/bin/free/dlmalloc.c` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 632 | garde-compilateur | condition simplifiée | `(defined(__GNUC__) && ((defined(__i386__) \|\| defined(__x86_64__)))) \|\| (defined(_MSC_VER) && _MSC_VER>=1310) → (defined(__i386__) \|\| defined(__x86_64__))` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 825 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 827 | garde-compilateur | branche morte (précédente toujours vraie) | `defined(_MSC_VER)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 832 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 834 | garde-compilateur | branche morte (précédente toujours vraie) | `defined(_MSC_VER)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 836 | garde-compilateur | branche morte (précédente toujours vraie) | `` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1470 | garde-compilateur | branche compilateur retirée | `defined(_MSC_VER) && _MSC_VER>=1300` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2791 | garde-compilateur | condition simplifiée | `defined(__GNUC__) && (defined(__i386__) \|\| defined(__x86_64__)) → (defined(__i386__) \|\| defined(__x86_64__))` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2820 | garde-compilateur | branche compilateur retirée | `defined(_MSC_VER) && _MSC_VER>=1300` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2895 | garde-compilateur | condition simplifiée | `defined(__GNUC__) && (defined(__i386__) \|\| defined(__x86_64__)) → (defined(__i386__) \|\| defined(__x86_64__))` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2911 | garde-compilateur | branche compilateur retirée | `defined(_MSC_VER) && _MSC_VER>=1300` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2992 | garde-compilateur | condition simplifiée | `defined(__GNUC__) && __GNUC__ >= 3 → __GNUC__ >= 3` |
| `sys/user/tauon-basic/src/bin/sdramtest/sdramtest_main.c` | 20 | pragma-iar | _Pragma location → __lepton_section | `EXT_RAM` |
| `sys/user/tauon-basic/src/bin/sdramtest/sdramtest_main.c` | 4 | pragma-iar | inclusion de compiler.h ajoutée | `#include "kernel/core/compiler.h"` |

## Code tiers non modifié — justifiés (58)

Décision D1a (2026-10-01) : le code vendored (CMSIS, HAL/driverlib ST, FatFs, yaffs…) n'est pas transformé ; ses branches IAR sont inactives sous GCC (en-têtes multi-compilateurs du fournisseur). Source : `audit-iar.csv`.

| Fichier | Ligne | Catégorie | Motif |
|---|---:|---|---|
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/arm_math.h` | 380 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/arm_math.h` | 488 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/arm_math.h` | 7283 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/arm_math.h` | 7298 | pragma | _Pragma optimize |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/arm_math.h` | 7305 | pragma | _Pragma optimize |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0.h` | 89 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0.h` | 125 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0plus.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0plus.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0plus.h` | 89 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0plus.h` | 125 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm3.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm3.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm3.h` | 89 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm3.h` | 125 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4.h` | 89 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4.h` | 137 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4_simd.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4_simd.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4_simd.h` | 669 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4_simd.h` | 671 | header | cmsis_iar.h |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cmFunc.h` | 610 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cmFunc.h` | 612 | header | cmsis_iar.h |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cmInstr.h` | 660 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cmInstr.h` | 662 | header | cmsis_iar.h |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc000.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc000.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc000.h` | 89 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc000.h` | 125 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc300.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc300.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc300.h` | 89 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc300.h` | 125 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f4xx.s` | 1 | asm_iar | IAR — démarrage, vecteurs |
| `sys/root/src/kernel/dev/arch/cmsis/dev_cmsis_itm/dev_cmsis_itm.h` | 32 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h` | 152 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h` | 174 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h` | 179 | mot_cle | __ramfunc |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h` | 200 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h` | 204 | pragma | _Pragma optimize |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4x7_eth.c` | 76 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4x7_eth.c` | 77 | pragma | #pragma data_alignment |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4x7_eth.c` | 79 | pragma | #pragma data_alignment |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4x7_eth.c` | 81 | pragma | #pragma data_alignment |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4x7_eth.c` | 83 | pragma | #pragma data_alignment |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_def.h` | 152 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_def.h` | 174 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_def.h` | 179 | mot_cle | __ramfunc |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_def.h` | 200 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_def.h` | 204 | pragma | _Pragma optimize |
| `sys/root/src/kernel/fs/fatfs/core/diskio.c` | 158 | mot_cle | __weak |
| `sys/root/src/kernel/fs/fatfs/fatfscore.c` | 70 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/fs/fatfs/fatfscore.c` | 78 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/fs/yaffs/core/yportenv.h` | 25 | garde_iar | __compiler_iar_arm__ |

