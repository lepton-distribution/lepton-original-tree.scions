# Graphe des dépendances entre composants (ETAPE-1, tâche 6)

Généré par `tools/migration/dep_graph.py` — ne pas éditer à la main. Graphe : `dependances.dot`
(`dot -Tsvg dependances.dot -o dependances.svg`). Intermédiaires (objets, journaux d'erreurs, stubs,
`results.json`) : `$LEPTON_BUILD/etape-1/depgraph/`.

## 1. Méthode

- Chaque fichier `.c` des composants est compilé **isolément** par le gcc hôte (`gcc -m32 -ffreestanding -nostdinc -fno-builtin -fpermissive -w -c`, sans édition de liens), puis `nm -P` donne les symboles globaux définis (T/D/B/R/C/W/V) et non résolus (U).
- `-m32` : le multilib gcc est présent mais **pas la libc 32 bits** (`libc6-dev-i386` absent) ; d'où `-ffreestanding -nostdinc` + en-têtes système minimaux générés (`stubs/sysinc` : string.h, stdlib.h, stdio.h, ctype.h, limits.h, math.h, setjmp.h, intrinsics.h…, simulant la DLib IAR).
- **Configuration simulée : celle du build IAR de référence**, pas une configuration GCC : `kal.h` n'a aucune branche GCC + embOS (seulement IAR/Keil), donc `-U__GNUC__ -D__ICCARM__ -D__IAR_SYSTEMS_ICC__=8 -D__CORE__=__ARM7EM__` ; extensions IAR neutralisées par `-include stubs/iar_compat.h` (`__no_init`, `__root`, `__ramfunc`, `__packed`, `__weak`…) ; defines des projets `tauon_8.40.ewp` / `dev_stm32f4xx_8.40.ewp` (`OS_LIBMODE_SP`, `OS_SUPPORT_CLEANUP_ON_TERMINATE`, `NDEBUG`) ; include paths de ces projets (embOS `embOSCXM4_518/inc`, CMSIS, lwIP, yaffs, uip2.5, cubemx HAL).
- `kernel/core/arch/cortexm/kernel_mkconf.h` (sortie mklepton, absente de l'arbre) est remplacé par un stub : STM32F4 / Cortex-M4, embOS, profil noyau *classic*, profil fs *full* (tous les fs référencés par le VFS), `__KERNEL_NET_IPSTACK` + `USE_LWIP` + `USE_IF_ETHERNET`, `STM32F429xx`. **HYPOTHÈSE À VALIDER** : valeurs reprises de `mkconf_tauon_basic_stm32f4_lwip.xml` et du `user_kernel_mkconf.h` discovery F4 ; pas d'en-tête STM32F439 dans cubemx, F429 retenu.
- Variante uIP : les fichiers de `kernel/core/net/uip_core`, `kernel/net/uip*`, `kernel/dev/arch/all/ppp` sont compilés avec `USE_UIP` au lieu de `USE_LWIP`.
- Fichiers non compilables : cause dominante relevée (première erreur), puis **extraction textuelle APPROXIMATIVE** : sur la sortie `gcc -E` (lignes du fichier principal) si le préprocesseur passe, sinon sur le texte brut ; définitions = fonctions non `static` et variables globales simples ; références = appels `f(` dans les corps + identificateurs des corps définis ailleurs. Les arêtes qui ne reposent que sur cette extraction sont signalées (colonne *texte*).
- Arête A → B : A référence (U) un symbole défini dans B et non dans A ; poids = nombre de symboles distincts. Un symbole défini dans plusieurs composants crée une arête vers chacun (voir §6).
- Aucune modification de source ; aucune écriture dans le trunk ni dans `scion/`.

### Périmètre

| Composant | Contenu |
|---|---|
| `bin` | `bin` |
| `kal` | `kernel/core/kal.c` (l'API KAL est surtout dans `kal.h`, macros/inline) |
| `kernel/core` | `kernel/core/*.c` hors `kal.c` |
| `kernel/core/core-generic` | `kernel_pthread_tsd.c` |
| `kernel/core/core-segger` | backend embOS |
| `kernel/core/net` | couche socket noyau (`lwip_core`, `uip_core`, `modem_core`) |
| `kernel/core/usb` | cœur USB STM32 (`stm32_usb_core`) |
| `kernel/dev (logiciel)` | `kernel/dev/dev_*` (null, proc, cpufs, head, tty, mem, fb, part, loadavg) |
| `kernel/dev/all` | `dev/arch/all` : circuits externes (eth, flash, i2c, lcd, modem, ppp, sd…) |
| `kernel/dev/bsp-stm32f4` | BSP discovery_f4, discovery_f4-baseboard-modem, olimex_p407, stm32f469i-eval |
| `kernel/dev/cmsis` | `dev/arch/cmsis` (cpu, ITM) — commun Cortex-M |
| `kernel/dev/stm32f4xx` | pilotes Lepton `dev/arch/cortexm/stm32f4xx` (hors HAL) |
| `kernel/dev/stm32f4xx-hal` | HAL ST vendored (`cubemx_hal_driver`, `driverlib`) |
| `kernel/fs/fat` | `kernel/fs/fat` |
| `kernel/fs/fatfs` | `kernel/fs/fatfs` |
| `kernel/fs/kofs` | `kernel/fs/kofs` |
| `kernel/fs/rootfs` | `kernel/fs/rootfs` |
| `kernel/fs/ufs` | `kernel/fs/ufs` |
| `kernel/fs/vfs` | `kernel/fs/vfs` |
| `kernel/fs/yaffs` | `kernel/fs/yaffs` |
| `kernel/net/lwip` | `kernel/net/lwip` |
| `kernel/net/uip` | `kernel/net/uip` |
| `kernel/net/uip2.5` | `kernel/net/uip2.5` |
| `kernel/usb` | pile USB device ST (`kernel/usb/stm32f4-usb-core`) |
| `lib/lib-nxpnfc` | `lib/lib-nxpnfc` |
| `lib/libc` | `lib/libc` |
| `lib/librt` | `lib/librt` |
| `lib/pthread` | `lib/pthread` |
| `sbin` | `sbin` |

Exclus (fichiers `.c`) :

- matériel Cortex-M hors STM32F4 (différé) : 195
- micro-noyau vendored (embOS/FreeRTOS/CMSIS) : hors graphe : 183
- gelé (bibliothèques Atmel ARM7/ARM9) : 151
- gelé (simulation Windows) : 29
- gelé (ARM9) : 24
- gelé (ARM7) : 16
- gelé (simulation Linux) : 16
- backend FreeRTOS (étape 7) : exclu, définitions concurrentes de core-segger : 15
- BSP hors STM32F4 (différé) : 14
- compris dans netif/ppp (doublon polarssl) : 5
- sorties mklepton win32 (gelé) : 3
- port lwIP synthétique (gelé) : 1
- port lwIP M16C (gelé) : 1
- port lwIP win32 (gelé) : 1

## 2. Taux de compilation par composant

Total : **689 / 826** fichiers compilés (83 %). *Vides* : objets compilés sans aucun symbole global (fichier exclu par la configuration, ex. pile réseau non retenue).

| Composant | Fichiers | Compilés | Taux | Vides | Causes d'échec dominantes |
|---|---:|---:|---:|---:|---|
| `bin` | 35 | 11 | 31 % | 0 | en-tête introuvable : kernel/signal.h (10), en-tête introuvable : kernel/libstd.h (7), en-tête introuvable : kernel/devio.h (2) |
| `kal` | 1 | 1 | 100 % | 1 | — |
| `kernel/core` | 26 | 25 | 96 % | 0 | en-tête introuvable : kernel/errno.h (1) |
| `kernel/core/core-generic` | 1 | 1 | 100 % | 0 | — |
| `kernel/core/core-segger` | 15 | 14 | 93 % | 0 | en-tête introuvable : memstruc.i (1) |
| `kernel/core/net` | 10 | 6 | 60 % | 0 | `static` après déclaration non static (toléré par IAR) (4) |
| `kernel/core/usb` | 3 | 3 | 100 % | 0 | — |
| `kernel/dev (logiciel)` | 11 | 8 | 73 % | 0 | identificateur non déclaré (config/macro) (2), en-tête introuvable : cyg/cpuload/cpuload.h (1) |
| `kernel/dev/all` | 51 | 34 | 67 % | 0 | identificateur non déclaré (config/macro) (8), type inconnu/incomplet (3), en-tête introuvable : atmel/ioat91sam9261.h (2) |
| `kernel/dev/bsp-stm32f4` | 26 | 7 | 27 % | 0 | identificateur non déclaré (config/macro) (10), `static` après déclaration non static (toléré par IAR) (8), type inconnu/incomplet (1) |
| `kernel/dev/cmsis` | 3 | 3 | 100 % | 0 | — |
| `kernel/dev/stm32f4xx` | 15 | 10 | 67 % | 1 | type inconnu/incomplet (3), identificateur non déclaré (config/macro) (2) |
| `kernel/dev/stm32f4xx-hal` | 121 | 115 | 95 % | 57 | type inconnu/incomplet (3), identificateur non déclaré (config/macro) (3) |
| `kernel/fs/fat` | 6 | 6 | 100 % | 0 | — |
| `kernel/fs/fatfs` | 16 | 9 | 56 % | 1 | #error de configuration (4), syntaxe (extension IAR ou config) (3) |
| `kernel/fs/kofs` | 1 | 1 | 100 % | 0 | — |
| `kernel/fs/rootfs` | 2 | 2 | 100 % | 0 | — |
| `kernel/fs/ufs` | 7 | 7 | 100 % | 0 | — |
| `kernel/fs/vfs` | 4 | 4 | 100 % | 1 | — |
| `kernel/fs/yaffs` | 18 | 15 | 83 % | 0 | conflit de déclarations (1), type inconnu/incomplet (1), en-tête introuvable : sys/stat.h (1) |
| `kernel/net/lwip` | 78 | 78 | 100 % | 46 | — |
| `kernel/net/uip` | 176 | 145 | 82 % | 4 | identificateur non déclaré (config/macro) (10), en-tête introuvable : ip64-conf.h (8), type inconnu/incomplet (5) |
| `kernel/net/uip2.5` | 102 | 89 | 87 % | 2 | identificateur non déclaré (config/macro) (3), type inconnu/incomplet (3), syntaxe (extension IAR ou config) (3) |
| `kernel/usb` | 19 | 16 | 84 % | 0 | type inconnu/incomplet (2), identificateur non déclaré (config/macro) (1) |
| `lib/lib-nxpnfc` | 11 | 11 | 100 % | 7 | — |
| `lib/libc` | 24 | 24 | 100 % | 2 | — |
| `lib/librt` | 2 | 2 | 100 % | 0 | — |
| `lib/pthread` | 3 | 3 | 100 % | 0 | — |
| `sbin` | 39 | 39 | 100 % | 1 | — |

Causes d'échec (toutes) :

- identificateur non déclaré (config/macro) : 41
- type inconnu/incomplet : 22
- `static` après déclaration non static (toléré par IAR) : 14
- en-tête introuvable : kernel/signal.h : 10
- en-tête introuvable : ip64-conf.h : 8
- en-tête introuvable : kernel/libstd.h : 7
- syntaxe (extension IAR ou config) : 6
- #error de configuration : 4
- autre : 3
- en-tête introuvable : kernel/devio.h : 2
- en-tête introuvable : atmel/ioat91sam9261.h : 2
- en-tête introuvable : kernel/types.h : 1
- en-tête introuvable : kernel/system.h : 1
- en-tête introuvable : kernel/errno.h : 1
- en-tête introuvable : memstruc.i : 1
- en-tête introuvable : dev_m16c_nju6433.h : 1
- en-tête introuvable : ioat91sam9261.h : 1
- en-tête introuvable : cyg/cpuload/cpuload.h : 1
- conflit de déclarations : 1
- en-tête introuvable : sys/stat.h : 1
- en-tête introuvable : cfs-coffee-arch.h : 1
- en-tête introuvable : dlfcn.h : 1
- en-tête introuvable : avr/boot.h : 1
- en-tête introuvable : dev/flash.h : 1
- en-tête introuvable : sys/mman.h : 1

Fichiers en échec : liste et journal par fichier dans `$LEPTON_BUILD/etape-1/depgraph/logs/`.

## 3. Matrice des dépendances (symboles)

Ligne = composant qui référence, colonne = composant qui définit. Abréviations :

C1=`bin`, C2=`kal`, C3=`kernel/core`, C4=`kernel/core/core-generic`, C5=`kernel/core/core-segger`, C6=`kernel/core/net`, C7=`kernel/core/usb`, C8=`kernel/dev (logiciel)`, C9=`kernel/dev/all`, C10=`kernel/dev/bsp-stm32f4`, C11=`kernel/dev/cmsis`, C12=`kernel/dev/stm32f4xx`, C13=`kernel/dev/stm32f4xx-hal`, C14=`kernel/fs/fat`, C15=`kernel/fs/fatfs`, C16=`kernel/fs/kofs`, C17=`kernel/fs/rootfs`, C18=`kernel/fs/ufs`, C19=`kernel/fs/vfs`, C20=`kernel/fs/yaffs`, C21=`kernel/net/lwip`, C22=`kernel/net/uip`, C23=`kernel/net/uip2.5`, C24=`kernel/usb`, C25=`lib/lib-nxpnfc`, C26=`lib/libc`, C27=`lib/librt`, C28=`lib/pthread`, C29=`sbin`

| | C1 | C2 | C3 | C4 | C5 | C6 | C7 | C8 | C9 | C10 | C11 | C12 | C13 | C14 | C15 | C16 | C17 | C18 | C19 | C20 | C21 | C22 | C23 | C24 | C25 | C26 | C27 | C28 | C29 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| C1 |  |  | 44 |  | 12 |  |  |  |  |  |  |  |  |  |  |  |  |  | 13 |  | 4 |  |  |  |  | 47 | 6 | 13 | 2 |
| C2 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| C3 |  |  |  |  | 27 | 1 |  |  |  |  |  |  |  |  |  |  |  |  | 6 |  |  | 4 | 2 |  |  | 4 |  |  |  |
| C4 |  |  |  |  | 3 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| C5 |  |  | 21 | 1 |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 46 |  |  |  |  |  |  | 2 |  |  |  |
| C6 |  |  | 10 |  | 12 |  |  |  | 1 |  |  |  |  |  |  |  |  |  | 4 |  | 33 | 19 | 19 |  |  | 2 |  |  |  |
| C7 |  |  | 3 |  |  |  |  |  |  |  |  |  | 2 |  |  |  |  |  | 1 |  |  |  |  | 11 |  |  |  |  |  |
| C8 |  |  | 3 |  | 7 |  |  |  |  |  |  |  |  |  |  |  |  |  | 8 |  |  |  |  |  |  | 1 |  |  |  |
| C9 |  |  | 9 |  | 14 | 6 |  |  |  |  | 2 |  |  |  |  |  |  |  | 6 |  |  | 6 | 7 |  |  | 3 |  |  |  |
| C10 |  |  | 2 |  | 2 |  |  |  | 2 |  |  | 21 | 22 |  |  |  |  |  | 1 |  |  |  |  |  |  |  |  |  |  |
| C11 |  |  |  |  | 1 |  |  |  |  |  |  |  |  |  |  |  |  |  | 1 |  |  |  |  |  |  |  |  |  |  |
| C12 |  |  | 2 |  | 4 |  |  |  |  |  |  |  | 101 |  |  |  |  |  | 1 |  |  |  |  |  |  | 1 |  |  |  |
| C13 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| C14 |  |  | 3 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 3 |  |  |  |  |  |  |  |  |  |  |
| C15 |  |  | 4 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 2 |  |  |  |  |  |  |  |  |  |  |
| C16 |  |  |  |  | 2 |  |  |  |  |  |  |  |  |  |  |  |  |  | 1 |  |  |  |  |  |  |  |  |  |  |
| C17 |  |  | 1 |  | 3 |  |  |  |  |  |  |  |  |  |  |  |  |  | 1 |  |  |  |  |  |  |  |  |  |  |
| C18 |  |  | 3 |  | 3 |  |  |  |  |  |  |  |  |  |  |  |  |  | 1 |  |  |  |  |  |  |  |  |  |  |
| C19 |  |  | 7 |  | 10 |  |  |  |  |  |  |  |  | 2 |  | 1 | 1 | 2 |  |  |  |  |  |  |  |  |  |  |  |
| C20 |  |  | 4 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 1 |  |  |  |  |  |  | 4 |  |  |  |
| C21 |  |  | 2 |  | 7 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| C22 |  |  | 3 |  | 1 | 4 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 7 |  |  |  |
| C23 |  |  |  |  | 1 | 3 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| C24 |  |  | 5 |  | 1 |  | 9 |  |  |  |  | 1 | 19 |  |  |  |  |  | 1 |  |  |  |  |  |  |  |  |  |  |
| C25 |  |  | 4 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 4 |  |  |  |
| C26 |  |  | 10 |  | 8 | 12 |  |  |  |  |  |  |  |  |  |  |  |  | 1 |  | 1 |  |  |  |  |  |  |  |  |
| C27 |  |  | 1 |  | 11 |  |  |  |  |  |  |  |  |  |  |  |  |  | 1 |  |  |  |  |  |  | 1 |  |  |  |
| C28 |  |  | 1 | 4 | 9 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| C29 |  |  | 43 |  | 6 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 39 |  |  |  |

### Arêtes principales (poids ≥ 5)

| De | Vers | Symboles | dont *texte* seul | Exemples |
|---|---|---:|---:|---|
| `kernel/dev/stm32f4xx` | `kernel/dev/stm32f4xx-hal` | 101 | 58 | `DMARxDscrTab`, `DMATxDescToSet`, `DMATxDscrTab`, `DMA_ClearITPendingBit`, `DMA_Cmd`, `DMA_DeInit` |
| `bin` | `lib/libc` | 47 | 7 | `__fclose`, `__fflush`, `__fgetc`, `__fgets`, `__fopen`, `__fprintf` |
| `kernel/core/core-segger` | `kernel/fs/vfs` | 46 | 0 | `_syscall_chdir`, `_syscall_close`, `_syscall_closedir`, `_syscall_creat`, `_syscall_fattach`, `_syscall_fdetach` |
| `bin` | `kernel/core` | 44 | 6 | `STDIN_FILENO`, `STDOUT_FILENO`, `_FD_ISSET`, `__gmtime`, `__localtime`, `__mktime` |
| `sbin` | `kernel/core` | 43 | 0 | `STDERR_FILENO`, `STDIN_FILENO`, `STDOUT_FILENO`, `_FD_ISSET`, `__ctime`, `__ctime_r` |
| `sbin` | `lib/libc` | 39 | 0 | `__fclose`, `__fflush`, `__fgetc`, `__fopen`, `__fprintf`, `__fputc` |
| `kernel/core/net` | `kernel/net/lwip` | 33 | 19 | `etharp_output`, `get_socket`, `lwip_accept`, `lwip_bind`, `lwip_close`, `lwip_connect` |
| `kernel/core` | `kernel/core/core-segger` | 27 | 1 | `__fds_size`, `__g_kernel_cpu`, `__g_kernel_desc_cpu`, `__g_kernel_desc_tty`, `__shl_fds_bits`, `_kernel_date` |
| `kernel/dev/bsp-stm32f4` | `kernel/dev/stm32f4xx-hal` | 22 | 21 | `EXTI_ClearITPendingBit`, `EXTI_GetITStatus`, `EXTI_Init`, `GPIO_Init`, `GPIO_PinAFConfig`, `GPIO_ReadInputDataBit` |
| `kernel/core/core-segger` | `kernel/core` | 21 | 0 | `__kernel_env`, `__mktime`, `_flocks`, `_is_locked`, `_mkbin`, `_put_flock` |
| `kernel/dev/bsp-stm32f4` | `kernel/dev/stm32f4xx` | 21 | 10 | `HAL_Delay`, `dev_stm32f4xx_i2c_x_load`, `dev_stm32f4xx_i2c_x_open`, `dev_stm32f4xx_sdio_close`, `dev_stm32f4xx_sdio_ioctl`, `dev_stm32f4xx_sdio_isset_read` |
| `kernel/core/net` | `kernel/net/uip2.5` | 19 | 3 | `tcpip_init`, `tcpip_input`, `uip_aligned_buf`, `uip_appdata`, `uip_conn`, `uip_connect` |
| `kernel/core/net` | `kernel/net/uip` | 19 | 3 | `tcpip_init`, `tcpip_input`, `uip_aligned_buf`, `uip_appdata`, `uip_conn`, `uip_connect` |
| `kernel/usb` | `kernel/dev/stm32f4xx-hal` | 19 | 0 | `HAL_Delay`, `HAL_GPIO_DeInit`, `HAL_GPIO_Init`, `HAL_NVIC_DisableIRQ`, `HAL_NVIC_EnableIRQ`, `HAL_PCD_DeInit` |
| `kernel/dev/all` | `kernel/core/core-segger` | 14 | 4 | `__g_kernel_desc_if_i2c_master`, `__g_kernel_desc_tty`, `__g_kernel_if_i2c_master`, `kernel_pthread_create`, `kernel_pthread_mutex_init`, `kernel_pthread_mutex_lock` |
| `bin` | `lib/pthread` | 13 | 0 | `pthread_cancel`, `pthread_cond_destroy`, `pthread_cond_init`, `pthread_cond_signal`, `pthread_cond_wait`, `pthread_create` |
| `bin` | `kernel/fs/vfs` | 13 | 13 | `_vfs_close`, `_vfs_ftruncate`, `_vfs_ls`, `_vfs_lseek`, `_vfs_makefs`, `_vfs_mkdir` |
| `bin` | `kernel/core/core-segger` | 12 | 3 | `_sys_getpid`, `kernel_pthread_self`, `kill`, `raise`, `rttmr_create`, `rttmr_start` |
| `kernel/core/net` | `kernel/core/core-segger` | 12 | 3 | `__g_kernel_desc_tty`, `kernel_mutex`, `kernel_pthread_create`, `kernel_pthread_mutex_init`, `kernel_pthread_mutex_lock`, `kernel_pthread_mutex_unlock` |
| `lib/libc` | `kernel/core/net` | 12 | 0 | `kernel_net_core_accept`, `kernel_net_core_accepted`, `kernel_net_core_bind`, `kernel_net_core_connect`, `kernel_net_core_gethostbyname`, `kernel_net_core_getpeername` |
| `kernel/core/usb` | `kernel/usb` | 11 | 0 | `HS_Desc`, `USBD_AUDIO`, `USBD_AUDIO_RegisterInterface`, `USBD_AUDIO_fops_HS`, `USBD_Init`, `USBD_MSC` |
| `lib/librt` | `kernel/core/core-segger` | 11 | 0 | `kernel_mutex`, `kernel_pthread_mutex_lock`, `kernel_pthread_mutex_unlock`, `kernel_pthread_self`, `kernel_sem_getvalue`, `kernel_sem_post` |
| `kernel/core/net` | `kernel/core` | 10 | 9 | `_sys_free`, `_sys_malloc`, `kernel_io_read`, `kernel_io_write`, `kernel_mqueue_flush`, `kernel_mqueue_get` |
| `kernel/fs/vfs` | `kernel/core/core-segger` | 10 | 0 | `__g_kernel_static_errno`, `_get_fd`, `_kernel_in_static_mode`, `_put_fd`, `_syscall_owner_pid`, `_syscall_owner_pthread_ptr` |
| `lib/libc` | `kernel/core` | 10 | 0 | `_system_free`, `_system_malloc`, `_system_sysctl`, `kernel_io_lseek`, `kernel_io_read`, `kernel_io_read_args` |
| `kernel/dev/all` | `kernel/core` | 9 | 0 | `_sys_free`, `_sys_malloc`, `kernel_io_ll_ioctl`, `kernel_io_ll_read`, `kernel_io_ll_write`, `kernel_io_read` |
| `kernel/usb` | `kernel/core/usb` | 9 | 0 | `g_usb_audio_core_info`, `g_usb_storage_core_info`, `krb_usb_audio_channel_source_input`, `usb_core_attr_configuration_string`, `usb_core_attr_interface_string`, `usb_core_attr_manufacturer_string` |
| `lib/pthread` | `kernel/core/core-segger` | 9 | 0 | `kernel_mutex`, `kernel_pthread_exit_cleanup`, `kernel_pthread_mutex_lock`, `kernel_pthread_mutex_trylock`, `kernel_pthread_mutex_unlock`, `kernel_pthread_self` |
| `kernel/dev (logiciel)` | `kernel/fs/vfs` | 8 | 7 | `_vfs_ioctl`, `_vfs_lseek`, `_vfs_mkdir`, `_vfs_mknod`, `_vfs_read`, `_vfs_stat` |
| `lib/libc` | `kernel/core/core-segger` | 8 | 0 | `_sys_getpid`, `kernel_mutex`, `kernel_pthread_alloca`, `kernel_pthread_mutex_lock`, `kernel_pthread_self`, `kernel_sem_wait` |
| `kernel/dev/all` | `kernel/net/uip2.5` | 7 | 1 | `stats`, `uip_aligned_buf`, `uip_appdata`, `uip_fw_register`, `uip_htons`, `uip_len` |
| `kernel/dev (logiciel)` | `kernel/core/core-segger` | 7 | 0 | `kernel_pthread_mutex_init`, `kernel_pthread_mutex_lock`, `kernel_pthread_mutex_unlock`, `kernel_pthread_self`, `kernel_sem_post`, `kernel_sem_wait` |
| `kernel/fs/vfs` | `kernel/core` | 7 | 0 | `_is_locked`, `_put_flock`, `_sys_free`, `_sys_malloc`, `_sys_unlockw`, `flock_lst` |
| `kernel/net/lwip` | `kernel/core/core-segger` | 7 | 0 | `kernel_clock_gettime`, `kernel_pthread_create`, `kernel_pthread_mutex_destroy`, `kernel_pthread_mutex_init`, `kernel_pthread_mutex_lock`, `kernel_pthread_mutex_unlock` |
| `kernel/net/uip` | `lib/libc` | 7 | 7 | `close`, `lseek`, `open`, `read`, `remove`, `tcflow` |
| `bin` | `lib/librt` | 6 | 0 | `_mq_open`, `_mq_timedreceive`, `_mq_timedsend`, `sem_init`, `sem_post`, `sem_wait` |
| `kernel/core` | `kernel/fs/vfs` | 6 | 0 | `_vfs_close`, `_vfs_mkfifo`, `_vfs_open`, `_vfs_read`, `_vfs_write`, `ofile_lst` |
| `kernel/dev/all` | `kernel/fs/vfs` | 6 | 3 | `_vfs_ioctl`, `_vfs_lseek`, `_vfs_open`, `_vfs_read`, `_vfs_write`, `ofile_lst` |
| `kernel/dev/all` | `kernel/core/net` | 6 | 0 | `modem_core_mq_post_unconnected_response`, `modem_core_parser_recv_at_response`, `modem_core_parser_send_at_command`, `modem_core_parser_send_recv_at_command`, `modem_core_parser_send_recv_at_command_ex`, `uip_hostaddr` |
| `kernel/dev/all` | `kernel/net/uip` | 6 | 0 | `uip_aligned_buf`, `uip_appdata`, `uip_fw_register`, `uip_htons`, `uip_len`, `uip_stat` |
| `sbin` | `kernel/core/core-segger` | 6 | 0 | `_kernel_date`, `_kernel_time`, `_sys_getpid`, `kernel_pthread_self`, `kill`, `sigaction` |
| `kernel/usb` | `kernel/core` | 5 | 0 | `kernel_io_ll_ioctl`, `kernel_io_ll_lseek`, `kernel_io_ll_read`, `kernel_io_ll_write`, `kernel_ring_buffer_read_min` |

## 4. Cycles (composantes fortement connexes)

### CFC 1 : 15 composants, 58 arêtes internes, 376 symboles en jeu

`kernel/core`, `kernel/core/core-generic`, `kernel/core/core-segger`, `kernel/core/net`, `kernel/dev/all`, `kernel/dev/cmsis`, `kernel/fs/fat`, `kernel/fs/kofs`, `kernel/fs/rootfs`, `kernel/fs/ufs`, `kernel/fs/vfs`, `kernel/net/lwip`, `kernel/net/uip`, `kernel/net/uip2.5`, `lib/libc`

Paires réciproques (A→B / B→A) :

| A | B | A→B | B→A | Exemples A→B | Exemples B→A |
|---|---|---:|---:|---|---|
| `kernel/core` | `kernel/core/core-segger` | 27 | 21 | `__fds_size`, `__g_kernel_cpu`, `__g_kernel_desc_cpu`, `__g_kernel_desc_tty` | `__kernel_env`, `__mktime`, `_flocks`, `_is_locked` |
| `kernel/core/core-segger` | `kernel/fs/vfs` | 46 | 10 | `_syscall_chdir`, `_syscall_close`, `_syscall_closedir`, `_syscall_creat` | `__g_kernel_static_errno`, `_get_fd`, `_kernel_in_static_mode`, `_put_fd` |
| `kernel/core` | `kernel/fs/vfs` | 6 | 7 | `_vfs_close`, `_vfs_mkfifo`, `_vfs_open`, `_vfs_read` | `_is_locked`, `_put_flock`, `_sys_free`, `_sys_malloc` |
| `kernel/core` | `lib/libc` | 4 | 10 | `__lepton_libc_isascii`, `__sprintf`, `ldiv`, `libc_lib_entrypoint` | `_system_free`, `_system_malloc`, `_system_sysctl`, `kernel_io_lseek` |
| `kernel/core/net` | `kernel/net/uip` | 19 | 4 | `tcpip_init`, `tcpip_input`, `uip_aligned_buf`, `uip_appdata` | `uip_draddr`, `uip_netmask`, `uip_sock_tcp_callback`, `uip_sock_udp_callback` |
| `kernel/core` | `kernel/net/uip` | 4 | 3 | `uip_arp_arpin`, `uip_arp_out`, `uip_len`, `uip_process` | `closedir`, `opendir`, `readdir` |
| `kernel/core/net` | `kernel/net/uip2.5` | 19 | 3 | `tcpip_init`, `tcpip_input`, `uip_aligned_buf`, `uip_appdata` | `uip_hostaddr`, `uip_sock_tcp_callback`, `uip_sock_udp_callback` |
| `kernel/core/core-segger` | `lib/libc` | 2 | 8 | `__printf`, `__stdio_init` | `_sys_getpid`, `kernel_mutex`, `kernel_pthread_alloca`, `kernel_pthread_mutex_lock` |
| `kernel/core/net` | `lib/libc` | 2 | 12 | `__sprintf`, `__sscanf` | `kernel_net_core_accept`, `kernel_net_core_accepted`, `kernel_net_core_bind`, `kernel_net_core_connect` |
| `kernel/fs/fat` | `kernel/fs/vfs` | 3 | 2 | `_vfs_getdesc`, `_vfs_putdesc`, `ofile_lst` | `fat_msdos_op`, `fat_vfat_op` |
| `kernel/core` | `kernel/core/net` | 1 | 10 | `socksconn_no` | `_sys_free`, `_sys_malloc`, `kernel_io_read`, `kernel_io_write` |
| `kernel/core/core-generic` | `kernel/core/core-segger` | 3 | 1 | `kernel_pthread_mutex_lock`, `kernel_pthread_mutex_unlock`, `process_lst` | `kernel_pthread_cleanup_specific` |
| `kernel/core/net` | `kernel/dev/all` | 1 | 6 | `trap_lion_flag` | `modem_core_mq_post_unconnected_response`, `modem_core_parser_recv_at_response`, `modem_core_parser_send_at_command`, `modem_core_parser_send_recv_at_command` |
| `kernel/fs/kofs` | `kernel/fs/vfs` | 1 | 1 | `ofile_lst` | `kofs_op` |
| `kernel/fs/rootfs` | `kernel/fs/vfs` | 1 | 1 | `ofile_lst` | `rootfs_op` |
| `kernel/fs/ufs` | `kernel/fs/vfs` | 1 | 2 | `ofile_lst` | `ufs_op`, `ufsx_op` |

Arêtes internes de faible poids (≤ 3 symboles : candidats à couper) :

- `kernel/core` → `kernel/core/net` : `socksconn_no`*
- `kernel/core` → `kernel/net/uip2.5` : `uip_len`*, `uip_process`*
- `kernel/core/core-generic` → `kernel/core/core-segger` : `kernel_pthread_mutex_lock`, `kernel_pthread_mutex_unlock`, `process_lst`
- `kernel/core/core-segger` → `kernel/core/core-generic` : `kernel_pthread_cleanup_specific`
- `kernel/core/core-segger` → `lib/libc` : `__printf`, `__stdio_init`
- `kernel/core/net` → `kernel/dev/all` : `trap_lion_flag`*
- `kernel/core/net` → `lib/libc` : `__sprintf`*, `__sscanf`
- `kernel/dev/all` → `kernel/dev/cmsis` : `dev_cmsis_itm_x_load`*, `dev_cmsis_itm_x_open`*
- `kernel/dev/all` → `lib/libc` : `libc_inet_addr`, `libc_ntohs`, `ltostr`
- `kernel/dev/cmsis` → `kernel/core/core-segger` : `kernel_sem_post`
- `kernel/dev/cmsis` → `kernel/fs/vfs` : `ofile_lst`
- `kernel/fs/fat` → `kernel/core` : `__mktime`, `__tm_conv`, `_sys_gettimeofday`
- `kernel/fs/fat` → `kernel/fs/vfs` : `_vfs_getdesc`, `_vfs_putdesc`, `ofile_lst`
- `kernel/fs/kofs` → `kernel/core/core-segger` : `g_pthread_lst`, `process_lst`
- `kernel/fs/kofs` → `kernel/fs/vfs` : `ofile_lst`
- `kernel/fs/rootfs` → `kernel/core` : `_sys_gettimeofday`
- `kernel/fs/rootfs` → `kernel/core/core-segger` : `__g_kernel_static_errno`, `_kernel_in_static_mode`, `_syscall_owner_pthread_ptr`
- `kernel/fs/rootfs` → `kernel/fs/vfs` : `ofile_lst`
- `kernel/fs/ufs` → `kernel/core` : `_sys_free`, `_sys_gettimeofday`, `_sys_malloc`
- `kernel/fs/ufs` → `kernel/core/core-segger` : `__g_kernel_static_errno`, `_kernel_in_static_mode`, `_syscall_owner_pthread_ptr`
- `kernel/fs/ufs` → `kernel/fs/vfs` : `ofile_lst`
- `kernel/fs/vfs` → `kernel/fs/fat` : `fat_msdos_op`, `fat_vfat_op`
- `kernel/fs/vfs` → `kernel/fs/kofs` : `kofs_op`
- `kernel/fs/vfs` → `kernel/fs/rootfs` : `rootfs_op`
- `kernel/fs/vfs` → `kernel/fs/ufs` : `ufs_op`, `ufsx_op`
- `kernel/net/lwip` → `kernel/core` : `_sys_free`, `_sys_malloc`
- `kernel/net/uip` → `kernel/core` : `closedir`, `opendir`, `readdir`
- `kernel/net/uip` → `kernel/core/core-segger` : `kernel_clock_gettime`
- `kernel/net/uip2.5` → `kernel/core/core-segger` : `kernel_clock_gettime`
- `kernel/net/uip2.5` → `kernel/core/net` : `uip_hostaddr`, `uip_sock_tcp_callback`, `uip_sock_udp_callback`
- `lib/libc` → `kernel/fs/vfs` : `ofile_lst`
- `lib/libc` → `kernel/net/lwip` : `lwip_htonl`

### CFC 2 : 2 composants, 2 arêtes internes, 20 symboles en jeu

`kernel/core/usb`, `kernel/usb`

Paires réciproques (A→B / B→A) :

| A | B | A→B | B→A | Exemples A→B | Exemples B→A |
|---|---|---:|---:|---|---|
| `kernel/core/usb` | `kernel/usb` | 11 | 9 | `HS_Desc`, `USBD_AUDIO`, `USBD_AUDIO_RegisterInterface`, `USBD_AUDIO_fops_HS` | `g_usb_audio_core_info`, `g_usb_storage_core_info`, `krb_usb_audio_channel_source_input`, `usb_core_attr_configuration_string` |

Arêtes internes de faible poids (≤ 3 symboles : candidats à couper) :

- (aucune)

## 5. Symboles non résolus dans tout le périmètre

Référencés (nm, fichiers compilés) mais définis dans aucun composant : fournis par la libc, embOS, la HAL/CMSIS, le runtime compilateur, les sorties mklepton ou l'application.

| Catégorie | Nombre | Exemples (composants demandeurs) |
|---|---:|---|
| autre (application/BSP/symbole manquant) | 58 | `get_bits_in_byte` (kernel/net/uip, kernel/net/uip2.5); `max_dev` (kernel/core/core-segger, kernel/fs/vfs); `rtimer_arch_init` (kernel/net/uip, kernel/net/uip2.5); `rtimer_arch_now` (kernel/net/uip, kernel/net/uip2.5); `rtimer_arch_schedule` (kernel/net/uip, kernel/net/uip2.5); `set_bits_in_byte` (kernel/net/uip, kernel/net/uip2.5); `slipdev_char_poll` (kernel/net/uip, kernel/net/uip2.5); `slipdev_char_put` (kernel/net/uip, kernel/net/uip2.5); `watchdog_periodic` (kernel/net/uip, kernel/net/uip2.5); `watchdog_start` (kernel/net/uip, kernel/net/uip2.5) |
| libc C standard (DLib/newlib) | 49 | `memcpy` (bin, kernel/core, kernel/core/core-segger, kernel/core/net, kernel/core/usb, kernel/dev (logiciel), kernel/dev/all, kernel/dev/bsp-stm32f4, kernel/dev/stm32f4xx, kernel/fs/fat, kernel/fs/fatfs, kernel/fs/rootfs, kernel/fs/vfs, kernel/fs/yaffs, kernel/net/lwip, kernel/net/uip, kernel/net/uip2.5, kernel/usb, lib/lib-nxpnfc, lib/libc, lib/pthread, sbin); `memset` (bin, kernel/core, kernel/core/core-segger, kernel/core/net, kernel/dev (logiciel), kernel/dev/all, kernel/fs/fat, kernel/fs/fatfs, kernel/fs/kofs, kernel/fs/rootfs, kernel/fs/ufs, kernel/fs/vfs, kernel/fs/yaffs, kernel/net/lwip, kernel/net/uip, kernel/net/uip2.5, kernel/usb, lib/lib-nxpnfc, lib/librt, sbin); `strcmp` (bin, kernel/core, kernel/core/core-segger, kernel/core/net, kernel/fs/fatfs, kernel/fs/kofs, kernel/fs/rootfs, kernel/fs/ufs, kernel/fs/vfs, kernel/fs/yaffs, kernel/net/lwip, kernel/net/uip, lib/libc, sbin); `strcpy` (bin, kernel/core, kernel/core/core-segger, kernel/dev/all, kernel/fs/kofs, kernel/fs/rootfs, kernel/fs/ufs, kernel/fs/vfs, kernel/fs/yaffs, kernel/net/uip, kernel/net/uip2.5, lib/libc, sbin); `strlen` (bin, kernel/core/core-segger, kernel/dev/all, kernel/fs/fat, kernel/fs/rootfs, kernel/fs/ufs, kernel/fs/vfs, kernel/fs/yaffs, kernel/net/lwip, kernel/net/uip, kernel/net/uip2.5, lib/libc, sbin); `memcmp` (bin, kernel/dev/all, kernel/fs/ufs, kernel/fs/yaffs, kernel/net/lwip, kernel/net/uip, kernel/net/uip2.5, lib/lib-nxpnfc); `isdigit` (bin, kernel/core, kernel/dev/all, kernel/dev/bsp-stm32f4, kernel/net/uip, lib/libc); `strncmp` (kernel/fs/fat, kernel/fs/yaffs, kernel/net/lwip, kernel/net/uip, lib/libc, sbin); `atoi` (bin, kernel/core/core-segger, kernel/dev/all, kernel/net/lwip, sbin); `memmove` (bin, kernel/fs/vfs, kernel/net/uip, kernel/net/uip2.5, sbin) |
| embOS | 42 | `OS_Global` (kernel/core, kernel/core/core-segger, kernel/core/net, kernel/dev/all, kernel/dev/stm32f4xx, kernel/fs/vfs, kernel/usb, lib/libc, lib/librt, lib/pthread); `OS_TASK_LeaveRegion` (kernel/core, kernel/core/core-segger, kernel/core/net, kernel/fs/vfs, lib/libc, lib/librt, lib/pthread); `OS_TASKEVENT_Set` (kernel/core, kernel/core/core-segger, kernel/fs/vfs, lib/libc, lib/librt, lib/pthread); `OS_TASKEVENT_Clear` (kernel/core, kernel/core/core-segger, lib/libc, lib/librt, lib/pthread); `OS_TASKEVENT_GetBlocked` (kernel/core, kernel/core/core-segger, lib/libc, lib/librt, lib/pthread); `OS_TASK_Delay` (kernel/core, kernel/dev/all, kernel/dev/bsp-stm32f4, kernel/dev/stm32f4xx); `OS_SwitchFromInt` (kernel/dev/all, kernel/dev/stm32f4xx, kernel/usb); `OS_SEMAPHORE_Create` (kernel/core/core-segger, kernel/net/lwip); `OS_SEMAPHORE_Delete` (kernel/core/core-segger, kernel/net/lwip); `OS_SEMAPHORE_Give` (kernel/core/core-segger, kernel/net/lwip) |
| intrinsèque/runtime compilateur | 4 | `__divdi3` (kernel/dev (logiciel), kernel/dev/all, kernel/dev/stm32f4xx, kernel/fs/fat, kernel/fs/kofs); `__moddi3` (kernel/dev/all, kernel/dev/stm32f4xx, kernel/fs/fat); `__udivdi3` (kernel/fs/fatfs, kernel/usb, lib/libc); `__umoddi3` (lib/libc) |
| généré par mklepton | 2 | `pdev_lst` (kernel/core/core-segger, kernel/dev/all, kernel/fs/vfs); `bin_lst` (kernel/core, kernel/core/core-segger) |
| HAL/CMSIS ST | 1 | `SystemCoreClock` (kernel/dev/stm32f4xx-hal) |

Liste complète : `$LEPTON_BUILD/etape-1/depgraph/unresolved.txt`. Appels non résolus issus de l'extraction textuelle (non comptés ici, bruit de macros) : 306.

## 6. Symboles définis dans plusieurs composants

| Composants | Symboles référencés | Exemples |
|---|---:|---|
| `kernel/net/uip`, `kernel/net/uip2.5` | 20 | `uip_conn`, `uip_len`, `uip_process`, `uip_udp_conn`, `uip_udp_conns` |
| `kernel/net/lwip`, `kernel/net/uip`, `kernel/net/uip2.5` | 2 | `tcpip_init`, `tcpip_input` |
| `kernel/dev/stm32f4xx`, `kernel/dev/stm32f4xx-hal` | 1 | `HAL_Delay` |

## 7. Proposition de découpage en bibliothèques statiques (étape 2)

### Robustesse des cycles

- Arêtes *nm* seules (sans extraction textuelle) : {`kernel/core`, `kernel/core/core-generic`, `kernel/core/core-segger`, `kernel/core/net`, `kernel/fs/fat`, `kernel/fs/kofs`, `kernel/fs/rootfs`, `kernel/fs/ufs`, `kernel/fs/vfs`, `kernel/net/lwip`, `kernel/net/uip`, `kernel/net/uip2.5`, `lib/libc`}; {`kernel/core/usb`, `kernel/usb`}.
- *nm* seules **et** sans les références du VFS aux tables d'opérations des fs (`*_op`, à générer par mklepton ou à enregistrer à l'initialisation) : {`kernel/core`, `kernel/core/core-generic`, `kernel/core/core-segger`, `kernel/core/net`, `kernel/fs/vfs`, `kernel/net/lwip`, `kernel/net/uip`, `kernel/net/uip2.5`, `lib/libc`}; {`kernel/core/usb`, `kernel/usb`}.

Contraintes : une bibliothèque par composant ; `bin`, `sbin`, `lib/*` hors de `kernel/`. Les cycles ci-dessus imposent, pour chaque CFC, soit une **fusion**, soit une édition de liens en groupe (`--start-group … --end-group`, en CMake `LINK_GROUP:RESCAN` ≥ 3.24 ou `target_link_libraries` circulaire entre bibliothèques STATIC, que CMake répète automatiquement).

| Bibliothèque CMake proposée | Composants | Dépend de (hors cycle) | Contrainte |
|---|---|---|---|
| `lepton_bin` | `bin` | `lepton_k_core`, `lepton_k_core_core_segger`, `lepton_k_fs_vfs`, `lepton_k_net_lwip`, `lepton_lib_libc`, `lepton_lib_librt`, `lepton_lib_pthread`, `lepton_sbin` | acyclique |
| `lepton_kal` | `kal` | — | acyclique |
| `lepton_k_core` | `kernel/core` | — | CFC 1 (groupe) |
| `lepton_k_core_core_generic` | `kernel/core/core-generic` | — | CFC 1 (groupe) |
| `lepton_k_core_core_segger` | `kernel/core/core-segger` | — | CFC 1 (groupe) |
| `lepton_k_core_net` | `kernel/core/net` | — | CFC 1 (groupe) |
| `lepton_k_core_usb` | `kernel/core/usb` | `lepton_k_core`, `lepton_k_dev_stm32f4xx_hal`, `lepton_k_fs_vfs` | CFC 2 (groupe) |
| `lepton_k_dev_sw` | `kernel/dev (logiciel)` | `lepton_k_core`, `lepton_k_core_core_segger`, `lepton_k_fs_vfs`, `lepton_lib_libc` | acyclique |
| `lepton_k_dev_all` | `kernel/dev/all` | — | CFC 1 (groupe) |
| `lepton_k_dev_bsp_stm32f4` | `kernel/dev/bsp-stm32f4` | `lepton_k_core`, `lepton_k_core_core_segger`, `lepton_k_dev_all`, `lepton_k_dev_stm32f4xx`, `lepton_k_dev_stm32f4xx_hal`, `lepton_k_fs_vfs` | acyclique |
| `lepton_k_dev_cmsis` | `kernel/dev/cmsis` | — | CFC 1 (groupe) |
| `lepton_k_dev_stm32f4xx` | `kernel/dev/stm32f4xx` | `lepton_k_core`, `lepton_k_core_core_segger`, `lepton_k_dev_stm32f4xx_hal`, `lepton_k_fs_vfs`, `lepton_lib_libc` | acyclique |
| `lepton_k_dev_stm32f4xx_hal` | `kernel/dev/stm32f4xx-hal` | — | acyclique |
| `lepton_k_fs_fat` | `kernel/fs/fat` | — | CFC 1 (groupe) |
| `lepton_k_fs_fatfs` | `kernel/fs/fatfs` | `lepton_k_core`, `lepton_k_fs_vfs` | acyclique |
| `lepton_k_fs_kofs` | `kernel/fs/kofs` | — | CFC 1 (groupe) |
| `lepton_k_fs_rootfs` | `kernel/fs/rootfs` | — | CFC 1 (groupe) |
| `lepton_k_fs_ufs` | `kernel/fs/ufs` | — | CFC 1 (groupe) |
| `lepton_k_fs_vfs` | `kernel/fs/vfs` | — | CFC 1 (groupe) |
| `lepton_k_fs_yaffs` | `kernel/fs/yaffs` | `lepton_k_core`, `lepton_k_fs_vfs`, `lepton_lib_libc` | acyclique |
| `lepton_k_net_lwip` | `kernel/net/lwip` | — | CFC 1 (groupe) |
| `lepton_k_net_uip` | `kernel/net/uip` | — | CFC 1 (groupe) |
| `lepton_k_net_uip25` | `kernel/net/uip2.5` | — | CFC 1 (groupe) |
| `lepton_k_usb` | `kernel/usb` | `lepton_k_core`, `lepton_k_core_core_segger`, `lepton_k_dev_stm32f4xx`, `lepton_k_dev_stm32f4xx_hal`, `lepton_k_fs_vfs` | CFC 2 (groupe) |
| `lepton_lib_lib_nxpnfc` | `lib/lib-nxpnfc` | `lepton_k_core`, `lepton_lib_libc` | acyclique |
| `lepton_lib_libc` | `lib/libc` | — | CFC 1 (groupe) |
| `lepton_lib_librt` | `lib/librt` | `lepton_k_core`, `lepton_k_core_core_segger`, `lepton_k_fs_vfs`, `lepton_lib_libc` | acyclique |
| `lepton_lib_pthread` | `lib/pthread` | `lepton_k_core`, `lepton_k_core_core_generic`, `lepton_k_core_core_segger` | acyclique |
| `lepton_sbin` | `sbin` | `lepton_k_core`, `lepton_k_core_core_segger`, `lepton_lib_libc` | acyclique |

### Analyse et proposition (rédigée sur la passe du 2026-09-30 ; à relire si les chiffres changent)

**Nœud du problème : un seul gros cycle « noyau + libc + réseau ».** Il ne tient pas à l'extraction
textuelle (il subsiste avec les arêtes *nm* seules). Ses mécanismes, du plus dense au plus ténu :

1. `kernel/core` ⇄ `kernel/core/core-segger` (≈ 27 / 21 symboles) et `core-segger` ⇄ `kernel/fs/vfs`
   (≈ 46 / 10 : les `_syscall_*` sont définis dans `vfskernel.c`, le VFS lit fd/errno/propriétaire dans le
   backend). Densité forte dans les deux sens : **indissociable** sans refactorisation.
2. `kernel/fs/vfs` → tables `*_op` de chaque fs (1–2 symboles) et fs → `ofile_lst`, `_vfs_*`. Cycle
   **artificiel** : la table des fs montés est codée en dur dans le VFS (profil fs). Coupure : table des fs
   générée (comme `dev_lst`/`bin_lst` par mklepton) → `fat`, `kofs`, `rootfs`, `ufs` deviennent acycliques.
3. `lib/libc` ⇄ noyau : libc → `kernel/core` / `core-segger` / `kernel/core/net` (≈ 10 + 8 + 12 : appels
   système, `kernel_io_*`, `kernel_net_core_*`) ; retour noyau → libc **7 symboles seulement** : `__sprintf`,
   `__sscanf`, `__printf`, `__stdio_init`, `ldiv`, `__lepton_libc_isascii`, `libc_lib_entrypoint`.
   Contrainte « `lib` hors de `kernel/` » : ce retour impose un groupe de liens transversal tant qu'il existe
   (coupure possible à l'étape 4 : formatage noyau via `kernel_printk`, point d'entrée libc enregistré).
4. `kernel/core/net` ⇄ piles IP : callbacks `uip_sock_*_callback`, `uip_hostaddr` définis dans
   `kernel/core/net` et appelés par uIP (3–4 symboles) ; lwIP ne revient que vers `kernel/core`
   (`_sys_malloc`) et `core-segger` (pthreads), et entre dans le cycle via `lib/libc` → `lwip_htonl`.
5. `kernel/dev/all` n'y entre que par une arête textuelle (`trap_lion_flag`) : acyclique en *nm*.

**Découpage proposé pour l'étape 2** (une bibliothèque STATIC par composant, cibles CMake) :

| Groupe de liens | Bibliothèques | Raison |
|---|---|---|
| G1 « noyau » (`LINK_GROUP:RESCAN`) | `k_core`, `k_core_segger` (ou `k_core_freertos` à l'étape 7), `k_core_generic`, `k_fs_vfs`, `k_core_net`, pile IP retenue (`k_net_lwip` **ou** `k_net_uip`), `lib_libc` | cycles 1, 3, 4 ; `core-segger` reste une bibliothèque séparée (axe micro-noyau interchangeable) |
| G1 tant que la table des fs n'est pas générée | `k_fs_fat`, `k_fs_kofs`, `k_fs_rootfs`, `k_fs_ufs` | cycle 2 ; sortent du groupe dès la génération de la table |
| fusion recommandée | `k_core_usb` + `k_usb` (≈ 11 / 9) | cycle dense entre glue Lepton et pile USB ST, toujours liés ensemble |
| acycliques (ordre de lien simple) | `sbin`, `bin` → `lib_pthread`, `lib_librt`, `lib_lib_nxpnfc` → G1 → `k_dev_sw`, `k_dev_bsp_stm32f4` → `k_dev_stm32f4xx`, `k_dev_all`, `k_dev_cmsis`, `k_fs_fatfs`, `k_fs_yaffs` → `k_dev_stm32f4xx_hal` (vendored) | pas de retour |
| pas de bibliothèque | `kal` | `kal.c` n'a de code que sous `CPU_WIN32` (objet vide sur cible) : le KAL est `kal.h` (macros/inline) + le backend |

Notes : les piles lwIP / uIP (contiki 3.0) / uIP 2.5 sont **mutuellement exclusives** (≈ 20 symboles
`uip_*` définis dans les deux uIP, `tcpip_init`/`tcpip_input` dans les trois) : une seule par image,
choix par option CMake. Les quatre BSP STM32F4 définissent aussi des symboles concurrents : une
bibliothèque BSP par carte. `bin`/`sbin` définissent des globales non `static` génériques (`byte`,
`offset`, `prompt`…) : collisions possibles dans une image statique unique (à surveiller à l'étape 3).

**Constats utiles hors graphe** (issus de la compilation) : `kal.h` n'a pas de branche GCC + embOS
(uniquement IAR/Keil ; et la condition de la branche embOS se termine par `|| (__tauon_cpu_core__ ==
cortexM7)` hors parenthèses, vraie pour tout compilateur sur M7) ; 14 fichiers redéclarent `static` une
fonction déjà déclarée non `static` dans leur en-tête (accepté par IAR, erreur GCC) ; inclusions
dépendantes de Windows (casse `RTOS.H`, `Legacy/`, séparateurs `\`), voir
`$LEPTON_BUILD/etape-1/depgraph/resolved-headers.tsv`.


## 8. Limites

- Configuration unique (STM32F4, embOS, lwIP, profil fs *full*) : le code sous `#if` d'autres configurations n'apparaît pas ; les objets *vides* le signalent. uIP compilé en variante séparée.
- Hôte x86 32 bits : asm inline ARM, intrinsèques non stubées et placements `@` empêchent la compilation ; ces fichiers ne contribuent que par extraction textuelle (approximative : macros non développées si le préprocesseur échoue, pointeurs de fonction et tables non vus).
- Symboles référencés via des tables générées (`dev_lst`, `bin_lst`… sorties mklepton) : invisibles ; les pilotes et commandes apparaissent donc « non référencés » par le noyau alors qu'ils le sont au link final.
- Les fonctions `static inline` des en-têtes (dont `kal.h`) sont compilées dans chaque utilisateur : la dépendance vers `kal` (API en macros/inline) est sous-estimée ; elle se traduit ici en références directes vers embOS (`OS_*`) et `core-segger`.
- En-têtes système simulés (DLib) : un conflit de déclarations peut provenir du stub et non du code.
