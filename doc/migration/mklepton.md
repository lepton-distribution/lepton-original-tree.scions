# mklepton — inventaire étape 1, tâche 5

Arbre lu : `trunk/tools/mklepton/`, `trunk/tools/bin/`, `mkconf*.xml` du trunk (2026-09-30).
Voir aussi `noyau-statique.md` (bibliothèque noyau liée) et `tools/migration/mklepton_oracle.sh`.

## 1. Sources (`tools/mklepton/src/`)

| Fichier | Lignes | Rôle | Plateforme |
|---|---|---|---|
| `mklepton.c` | 2602 | analyse XML (expat, `start`/`end`), génération des fichiers C, création de l'image UFS via le noyau statique (`_vfs_*`) | Linux/POSIX (`open/read/opendir/dirname`) |
| `mklepton.h` | 286 | balises/attributs XML, structures de listes (`kernel_conf_t`, `boot_t`, `mount_t`, `mkdev_t`, `mkbin_t`, `mkfile_t`, `mkdir_t`), masques d'options | commun |
| `kernel_stub.h` | 175 | **copie à la main** des prototypes/types noyau utilisés (`_vfs_*`, `_kernel_warmup_*`, `exec_file_t`, `_vfs_formatopt_t`, `k_tm`, `HDGETSZ/HDSETSZ`) pour éviter le conflit glibc/types Lepton | Linux |
| `mklepton-w32.c` | 2524 | variante Windows : inclut les vrais en-têtes noyau et `kernel/core/ucore/embOSW32_100/win32/windows.h`, `<conio.h>`, `<io.h>` ; gère `include_absolute_path` ; pas de `<directories>` | Windows (MSVC) |

- Dépendances de `mklepton_gnu` (`nm -D`) : glibc (≈40 fonctions), **libexpat** (`XML_*`), et
  **libkernel** : `_vfs`, `_vfs_open/close/write/ioctl/mkdir/mount/umount/makefs/statvfs/ls`,
  `_kernel_warmup_rootfs/dev/rtc/mount`, `__mktime`, `__wrpr_kernel_dev_gettime`, `xtime`.
- Build : `tools/mklepton/prj/scons/SConstruct` — `gcc -g -O0 -Wall`, `-lexpat -L<x86_static/bin>
  -lkernel -Wl,-rpath,<…>` avec `base_dir = $HOME/tauon/` codé en dur ; `platform.dist()` (Python 2,
  supprimé en Python 3.8). Projets Windows : `prj/vc/mklepton.dsp`, `prj/vc-2010/` (+ `expat_static`).
- Défauts relevés (à corriger au portage, pas maintenant) : `xml_elmt_mklepton` alloue
  `strlen(a)+strlen(b)+1` pour `"%s/%s"` (débordement d'un octet, `mklepton.c:1242-1265`) ;
  troncatures `strncat(…,32)` dans `_mk_dir` ; option `-r` non analysée dans `main` (seulement via
  `-a`/défaut) ; `kernel_stub.h` désynchronisé du noyau (`noyau-statique.md` §4) ; `_mk_rtc` exige
  `/dev/rtc0|1`, sinon échec global.

## 2. Binaires (`tools/bin/`)

| Fichier | Nature | Détails |
|---|---|---|
| `mklepton_gnu` | ELF 32 bits i386, dynamique, non strippé, debug | interpréteur `/lib/ld-linux.so.2` ; NEEDED `libexpat.so.1`, **`libkernel.so`**, `libc.so.6` ; RPATH `/home/sqzwork/tauon/sys/root/prj/scons/arch/synthetic/x86_static/bin` ; GLIBC_2.0/2.1 ; sha256 `9111eea6…c4` |
| `mklepton_gnu.sh` | bash | `sed "s:$(HOME):$HOME:g" $3 > /tmp/mkconf.xml.tmp` puis `./mklepton_gnu $1 $2 /tmp/mkconf.xml.tmp` (relatif au cwd) |
| `mklepton.exe` | PE32 i386 console | + `msvcr100d.dll` (CRT debug VS2010) ; sha256 `80fa9902…2e` ; produit des images **pack(1)** (MSVC ≠ GCC), inutilisables comme oracle pour un noyau GCC |

Prérequis de chargement (`ldd`, constat session principale 2026-09-30) :

| Bibliothèque | État | Remède |
|---|---|---|
| `/lib/ld-linux.so.2` (libc6:i386) | installé | — |
| `libexpat.so.1` i386 | `not found` | `apt install libexpat1:i386` (multiarch i386 déjà activé) |
| `libkernel.so` i386 (partagée) | `not found` ; **introuvable** dans le trunk, le clone (`depots/`) et le paquet `~/Development/lepton-migration-development` (`find -L`, 2026-09-30) | à fournir (copie historique validée) ou à reconstruire — voir §5 |

Pas d'en-têtes/bibliothèques 32 bits de développement (`gcc -m32` compile en freestanding mais ne lie pas).

## 3. Entrées

Ligne de commande : `mklepton_gnu [-a|-b|-d|-f|-k] [-t <cible>] <mkconf.xml>` (défaut
`mkconf.xml` du cwd ; sans option ou avec `-t` seul : tout). `-t` filtre les blocs `<target name=…>` ;
les éléments hors `<target>` sont toujours traités.

Schéma XML (balises lues par `mklepton.c`, attributs utilisés dans l'arbre) :

```
<mklepton dest_path>                      racine ; dest_path des fichiers C générés
  <target name>                           filtre -t
    <arch dest_path [include_absolute_path(w32)]>   remplace dest_path (idem mklepton)
    <kernel>                              -k : kernel_mkconf.h
      <cpu type freq/> <heap size/> <thread max/> <process max/> <openfiles max/>
      <descriptors max/> <env path="a;b"/> <network use/>
      <cpufs size node blocksz [option="-split"]/>   géométrie de l'image UFS
      <boot dest_path dev delay> <command value arg/>… </boot>   -> .boot
      <mount dest_path> <disk type dev point/>… </mount>         -> .mount
    <devices> <dev name use="on|off"/>… </devices>  -d : dev_mkconf.c
    <binaries src_path dest_path> <bin name priority stack timeslice/>… </binaries>  -b
    <files> <file src_file name dest_path/>… </files>   -f : fichiers hôte copiés dans l'image
    <directories> <directory src_path dest_path/>… </directories>  (gnu seulement) copie récursive
```

Inventaire des mkconf du trunk (26 fichiers ; détail : `$LEPTON_BUILD/etape-1/mklepton/scan_mkconf.out`) :

| Emplacement | Nb | Cibles | `dest_path` de sortie |
|---|---|---|---|
| `sys/user/tauon-basic/etc/mkconf_tauon_basic*.xml`, `mkconf_tauon_uip_*.xml` | 17 | `win32_lepton` + `cortexm_lepton`/`cmsis_lepton`/`arm7_lepton`/`arm9_lepton` | `c:/tauon/sys/root/src/kernel/core/arch/{cortexm,arm,win32}` |
| `sys/user/tauon-basic/etc/mkconf_base.xml`, `mkconf_complet.xml` | 2 | `gnu32_lepton`, `arm9_lepton` | `/home/pitrolle/tauon/src/…`, `~/tauon/src/…` (ancienne arborescence sans `sys/root`) |
| `sys/user/tauon_sampleapp/etc/mkconf_tauon_sampleapp_*.xml` | 4 | `gnu32_lepton`, `arm9_lepton`, `cortexm_lepton` | `$(HOME)/tauon/sys/root/src/kernel/core/arch/{arm,cortexm,synthetic/x86}` |
| `tools/mklepton/mkconf.xml` | 1 | gnu32/arm7/arm9 | `/home/shiby/tauon/…`, `/opt/lepton/…` |
| `tools/mklepton/mkconf_9260.xml` | 1 | gnu32/arm7/arm9 | `../config/{arm7,arm9,x86}` (relatif) |
| `tools/mklepton/prj/vc/mkconf.xml` | 1 | aucune | `x:/sources/kernel/config` |

Configurations les plus proches du F439 : `mkconf_tauon_basic_stm32f4*.xml` (`cortexm_lepton`,
cpufs 64000 o / 256 nœuds / blocs 256). Contenu du rootfs référencé : `.boot`, `.init*`, `.mount`
(`sys/user/*/etc/`), scripts `src/sh/*.sh`, pages `net/html`, `cgi-bin/*.sh` ; certains chemins
source sont extérieurs à l'arbre (`/opt/elog-web/etc/conf_idx.json`, `x:/sources/…`).

## 4. Sorties exactes

| Sortie | Emplacement | Producteur |
|---|---|---|
| `kernel_mkconf.h` | `<dest_path>/` (`<mklepton>` ou `<arch>`) | `xml_elmt_start/end_kernel` ; inclut `"dev_dskimg.h"` ; `__KERNEL_CPU_FREQ`, `__KERNEL_HEAP_SIZE`, `__KERNEL_PTHREAD_MAX`, `__KERNEL_PROCESS_MAX`, `MAX_OPEN_FILE`, `OPEN_MAX`, `__KERNEL_ENV_PATH`, `__KERNEL_NET_IPSTACK` |
| `dev_mkconf.c` | `<dest_path>/` | `_mk_dev` : `extern dev_map_t …; pdev_map_t const dev_lst[]` |
| `bin_mkconf.c` | `<dest_path>/` | `_mk_binaries` : `bin_t _bin_lst[]` (nom, `<nom>_main`, priorité, pile, timeslice) |
| `dev_dskimg.c` / `dev_dskimg.h` | `<dest_path>/` | `_mk_dskimg` : `const unsigned char filecpu_memory[]` = octets de `.fsflash.o` (25 par ligne, CRLF) ; `-split` : pragmas IAR M16C |
| `.boot`, `.mount` | `<boot dest_path>/`, `<mount dest_path>/`, sinon **cwd** | `_mk_boot`, `_mk_mount` (texte) |
| `.fsflash.o` | **cwd** | image UFS brute de `/dev/hd/hdc` (`dev_linux_fileflash`) : makefs + `/usr/<bin dest>/…` (`exec_file_t`) + fichiers + répertoires |
| `.fsrom.o` | **cwd** (si ouvert au warmup) | `dev_linux_filerom` |

Chemins `dest_path` : absolus d'anciens postes, `$(HOME)/tauon/…` (substitué par
`mklepton_gnu.sh`) ou relatifs. Tous visent `…/sys/root/src/kernel/core/arch/<arch>/`, donc **le
trunk** une fois `$HOME/tauon` → rootstock : conflit ouvert (CLAUDE.md), à trancher à l'étape 2
(proposition : sorties dans `$LEPTON_BUILD/<preset>/generated/`, include par CMake). Les images
contiennent l'horodatage de l'hôte (`cmtime`) : non reproductibles octet pour octet.

## 5. Verdict et oracle

- **Portage natif (voie nominale) : faisable.** `mklepton.c` est du C POSIX + expat, sans
  dépendance i386 ; les sources complètes sont présentes. Le travail porte sur (1) le noyau statique
  (`noyau-statique.md` : backend statique à recréer, `core-ecos/` et `x86_static/` absents),
  (2) le remplacement de `kernel_stub.h` par une interface partagée avec le noyau, (3) un pilote disque
  sur fichier hôte et une horloge déterministe (option `SOURCE_DATE_EPOCH` suggérée),
  (4) les chemins de sortie. Aucun repli n'est nécessaire.
- **Oracle** : `mklepton_gnu` ne peut pas s'exécuter sans `libkernel.so` (absente du trunk, du clone
  et du paquet de migration). Trois voies, **décision utilisateur** :
  1. copie historique repérée hors arbre (`/mnt/hgfs/entreprises/lepton/…/prj/scons/arch/synthetic/x86_static/bin/libkernel.so`,
     5 copies identiques, sha256 `aae48adb…`, 904 971 o, ELF i386 partagé) — provenance et
     correspondance avec le code actuel à valider (elle embarque l'ancien `core-ecos` et un noyau
     antérieur ; la lecture détaillée a été bloquée pour l'agent) ;
  2. reconstruction i386 depuis l'arbre actuel : **impossible en l'état** (`core-ecos/`,
     `x86_static/`, en-têtes eCos absents) et équivalente au portage du noyau statique de l'étape 2 —
     ce ne serait plus un oracle indépendant ;
  3. repli sans exécution : sorties C déterministes dérivées des mkconf (format connu, §4) et sorties
     versionnées `kernel/core/arch/win32/{kernel_mkconf.h,dev_mkconf.c,bin_mkconf.c,dev_dskimg.c}`
     (image au format MSVC pack(1), non comparable octet à octet) ; l'image UFS est alors validée par
     exécution (montage sous QEMU, étape 3) plutôt que par comparaison.
  Faisabilité de l'oracle binaire : **conditionnée à la voie 1**.
- `tools/migration/mklepton_oracle.sh` (+ `mklepton_oracle_rewrite.py`) : bacs à sable
  `$LEPTON_BUILD/etape-1/mklepton-ref/<mkconf>/<cible>/{home,work,out}`, réécriture des `dest_path`
  de sortie et de `$(HOME)`, sources remappées en lecture vers le trunk, garde-fou (refus si un
  chemin résolu est sous le trunk, le clone ou `$HOME/tauon`), log de la commande, `rc`,
  `sha256sums.txt`, `outputs.tar`, `summary.csv`, `run-info.txt`. Testé en `--dry-run` (55 couples
  mkconf × cible) et garde-fous vérifiés (`--out` dans le trunk refusé ; arrêt propre sans prérequis).

Commande (session principale, après installation des prérequis) :

```bash
sudo apt install libexpat1:i386        # libc6:i386 déjà installé
tools/migration/mklepton_oracle.sh --libkernel-dir <répertoire contenant libkernel.so validé> \
    [--targets gnu32_lepton,cortexm_lepton] [--only tauon_basic_stm32f4]
```
