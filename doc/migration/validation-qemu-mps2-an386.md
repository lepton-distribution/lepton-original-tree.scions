# Journal de validation — QEMU mps2-an386 (étape 3)

Machine : `qemu-system-arm -M mps2-an386` (QEMU 10.0.13), Cortex-M4F, embOS-Classic V5.20.0.0,
`arm-none-eabi-gcc` 14.2.1, `-Og -g` (Debug), newlib-nano. Deux presets (décision 2026-09-30) :

| Preset | ABI | Bibliothèque embOS | `lepton.elf` (text / data / bss) |
|---|---|---|---|
| `qemu-mps2-an386-embos` (principal) | hard (`fpv4-sp-d16`) | `libosT7VHLSP.a` | 397 732 / 1 504 / 128 216 (avec lwIP, 3b) |
| `qemu-mps2-an386-embos-soft` | soft | `libosT7LSP.a` | 396 788 / 1 504 / 127 672 (avec lwIP, 3b) |

Paliers 1 à 6 d'abord validés en soft-float, puis rejoués en hard-float (2026-09-30) : fumée et
banc KAL verts dans les deux presets (`ci/run.sh`).

## Paliers (ordre ETAPE-3, arrêt au premier échec)

| Palier | Statut | Date | Preuve |
|---|---|---|---|
| 1. Reset → `main` | VERT | 2026-09-30 | gdb (`qemu -s -S`) : PC initial `Reset_Handler`, SP `0x20400000` (fin de SSRAM2), arrêt sur `main` |
| 2. `.data`, `.bss`, `.noinit` | VERT (indirect) | 2026-09-30 | amorçage complet et variables initialisées correctes (`SystemCoreClock`, tables) ; pas de test dédié de `.noinit` — à couvrir par le banc KAL (T0) |
| 3. Warmup du noyau | VERT | 2026-09-30 | gdb : `_start_kernel` → `_kernel_warmup_boot` atteint ; rootfs, cpufs `/usr` (image mklepton), pilotes, console `/dev/console` ; `ls /` : `dev kernel bin usr etc var mnt` |
| 4. Premier appel système tracé | VERT | 2026-09-30 | gdb (`qemu -s -S`, hard-float), trace archivée `traces/palier4-appel-systeme-hard.txt` (script `traces/palier4-appel-systeme.gdb`) : `initd` → `_system_setpgid` → `kernel_syscall_lock` ; tâche `kernel_thread` réveillée par événement embOS (pas de SVC) → `_kernel_syscall` (pid 1, syscall 49) → `_syscall_setpgid` (retour 0) → `kernel_syscall_unlock` (état `END`) ; `initd` reprend ligne suivante, errno 0 |
| 5. Multitâche | VERT (niveau KAL) | 2026-09-30 | `ps` : initd, lsh, ps (création de processus, attente, `exit`) ; banc KAL `ctest -L kal` : préemption par priorité, déroutement de signaux T4-T6, vfork T7, exec T8, variantes FPU T1F/T4F/T6F/T7F (hard-float) ; signaux de bout en bout (T9-T11) : pseudo-binaires `bin` à écrire |
| 6. Fumée canonique | VERT | 2026-09-30 | `ctest -L smoke` : `uname -a` = `lepton-cortexm4-32 4.10.0.2 … cortexM4-qemu-mps2-an386`, `ls`, `ps`, UART1 (`echo … > /dev/ttys1`) |
| 7. Réseau | VERT | 2026-09-30 | `ctest -L net` (`net.ping_ftpd`, `tests/net_qemu.py`), hard et soft : tap dans un espace de noms (`unshare -rn`, sans sudo), `-nic tap` sur le LAN9118 ; ping hôte → invité 3/3 (RTT ≈ 0,3 ms : réception par interruption, pas par l'attente périodique de 100 ms d'`ethif_core`) ; session FTP (`tauon`) : bannière 220, `CWD /usr/sbin` + `LIST` (13 entrées, `lsh`), `RETR /usr/etc/.boot` identique à la source (57 octets) ; `ftpd` toujours présent dans `ps` ; contrôle négatif : adresse invitée fausse → échec 0/3 |

## Défauts trouvés et corrigés pendant la mise au point

| Symptôme | Cause | Correction |
|---|---|---|
| `OS_Error(OS_ERR_MUTEX_OWNER)` dès le premier appel système | verrou des appels système (mutex embOS) pris par l'appelant, rendu par la tâche noyau ; embOS 5.20 le refuse (DP) ou le laisse incohérent (SP) | sémaphore (`core-segger/kernel_syscall_lock.c`), décision 2026-09-30 |
| Noyau arrêté (`OS_TerminateError`) : processus 1 terminé (`ENOENT`) | `.boot` lançait `initd` sur `/dev/ttys0`, déjà pris par la console | `.boot` QEMU : `initd -i /dev/console -o /dev/console` (comme Olimex) |
| Blocage après la bannière d'initd | contrat `kernel_io` : l'écrivain attend l'interruption de fin d'émission ; pilote UART synchrone sans signalement | `dev_cmsdk_uart_x_write` : `__fire_io_int` de fin d'émission |
| Hard-float : gestionnaire de signal et fils de vfork jamais exécutés, S16-S31 non restaurés (T1F, T4F, T6F, T7F rouges ; T1-T8 verts) | E3 : cadre embOS étendu `OS_REGS_BASE_FPU` pour une tâche à contexte FPU actif ; `kal.h` écrivait le PC au décalage du cadre de base (dans S16-S31) et copiait 72 octets au lieu de 208 | `kal.h` : `context_t.os_regs` = union `OS_REGS`, taille et PC selon le bit 4 d'`OS_REG_EXC_RETURN` |

| 3b : `ifconfig` du `.init` en échec (statut 255) au démarrage | ligne de commande d'un processus copiée dans `arg[ARG_LEN_MAX]` = 64 octets (`process.h`), **troncature silencieuse** : `255.255.255.0` reçu `255.255.255` (gdb) | ligne du `.init` raccourcie (masque par défaut, comme Olimex) ; limite consignée en dette |
| 3b : `ftpd` lancé par le `.init` absent de `ps` | `socket()` avant l'initialisation de lwIP : tâche `lwip_core` en priorité 10 sous `__GNUC__` (branche de la simulation Linux) < processus (100) ; `inited = 0`, boîte `tcpip` invalide (gdb) | branches `#if !defined(__GNUC__)` retirées : `lwip_core` 120, sessions `ftpd` 100 (valeurs IAR des cibles réelles) |
| 3b : compilation lwIP / `ftpd` / `lwip_core_socket.c` | `inttypes.h` hors des en-têtes autorisés (`cc.h` lwIP 1.x) ; prototypes non `static` avant définitions `static` ; déclarations implicites (ctype, `strdup`, `strerror`) et `int*` pour `socklen_t*` (erreurs GCC 14) | `cc.h` (`LWIP_NO_INTTYPES_H`, formats) ; prototypes `static` ; en-têtes Lepton ; `strdup`/`strerror` ajoutés à la libc Lepton (décision 2026-09-30) |

## Hypothèses validées / restant à valider

- VALIDÉE : UART CMSDK : interruptions RX sur les lignes 0-5/12 routées (réception de `lsh` fonctionnelle).
- VALIDÉE : LAN9118 à `0x40200000`, IRQ 13 (source QEMU v10.0.0 `hw/arm/mps2.c`) : ping et FTP par
  interruption. Registres d'après le modèle QEMU (`hw/net/lan9118.c`), sans fiche technique SMSC :
  pilote validé sur QEMU seulement (la carte réelle MPS2 porte un LAN9220, hors cible).
- VALIDÉE (fonctionnellement) : `OS_MakeTaskReady(OS_TASK*)` (E4) — exercée par T4-T8 ; signature toujours non documentée par Segger.
- CONSTAT : cadre de départ embOS 5.20 = `OS_REGS_BASE` + routine et retour `OS_StartTask` au-dessus (T2).
- VALIDÉE : trame FPU (E3) — cadre étendu détecté par le bit 4 d'EXC_RETURN ; T1F/T4F/T6F/T7F verts ;
  lazy stacking actif (FPCCR ASPEN et LSPEN, QEMU).
- CONSTAT (sécurité) : après `exec`, la nouvelle image démarre sans contexte FPU (FPCA = 0, cadre de
  départ de base, FPSCR par défaut `0x02000000`), mais S16-S31 contiennent encore les valeurs de
  l'ancienne image (T8, journalisé). Banc de registres FPU physique et partagé, non effacé par embOS :
  fuite d'information entre tâches et entre images, sans conséquence supplémentaire tant que Lepton
  n'isole pas la mémoire (tout processus lit déjà toute la RAM) ; à traiter avec l'isolation par MPU
  (dette de sécurité, `MIGRATION-STATUS.md`).
