# Inventaire des paquets embOS (étape 1, tâche 0)

Produit le 2026-09-30. Sources : paquet `$LEPTON_EMBOS_ROOT` (lecture seule), release notes HTML,
UM01039 (62 p.), UM01001 (633 p.) convertis par `pdftotext` dans `$LEPTON_BUILD/etape-1/kal/`.
Script rejouable : `tools/migration/embos_inventory.py` (API → `embos-api.txt`, CSV intermédiaires
dans `$LEPTON_BUILD/etape-1/kal/`). Aucun code Segger n'est recopié ici.

## 1. Paquets présents

| Famille | Chemin | Version | Statut |
|---|---|---|---|
| Cortex-M GCC | `third_party/embos/cortexm-gcc/5.20.0.0/` | **embOS-Classic V5.20.0.0 for Cortex-M and GCC**, 26 mars 2025 (release notes) ; `OS_VERSION_GENERIC 52000`, `OS_PORT_REVISION 0` (RTOS.h) | présent |
| RISC-V | — | — | **absent** : l'annexe RISC-V le dit « déjà téléchargé », il n'est pas dans `third_party/embos/`. À fournir par l'utilisateur (non bloquant : portage reporté). |

Chaîne de construction des bibliothèques (release notes) : Arm GNU Toolchain 13.2.rel1
(`arm-none-eabi-gcc 13.2.1 20231009`, binutils 2.41). Les `.a` portent DWARF 4 (depuis 5.18.0.2).

## 2. Contenu

| Élément | Emplacement | Remarque |
|---|---|---|
| API | `Start/Inc/RTOS.h` (4115 l.) | ne compile qu'avec un `OS_LIBMODE_*` défini (sinon `#error`) |
| Configuration | `Start/Inc/OS_Config.h` | `DEBUG=1` → `OS_LIBMODE_DP`, sinon `OS_LIBMODE_R` + embOSView désactivé |
| Autres en-têtes | `BSP.h`, `BSP_UART.h`, `Global.h`, `JLINKMEM.h` | interfaces BSP d'exemple |
| Bibliothèques | `Start/Lib/libosT*.a` : **378** fichiers (99 Mo), 34 objets chacune | voir §3 |
| Handlers système | **`PendSV_Handler` est dans la bibliothèque** (objet `OS_ARMv7M_ISR.o`, relevé par `nm`) ; **`SysTick_Handler` est dans `RTOSInit_*.c`** (source BSP, C) ; aucun `SVC_Handler` | pas de `.S` Segger pour PendSV/SysTick |
| Asm GNU fournis | `HardFaultHandler.S` (67 copies), `SEGGER_RTT_ASM_ARMv7M.S` (59), startups `.s/.S` par carte (≈50) | les startups sont d'origine fabricant (ST, NXP…) |
| Scripts d'édition de liens | 73 `.ld` (un ou plusieurs par BSP) | définissent `__stack_start__`/`__stack_end__` requis par la lib |
| BSP | `Start/BoardSupport/<fabricant>/<carte>/` : **69** projets (emIDE `.emP`, parfois SW4STM32/CubeIDE `.cproject`) | + `CMSIS/Generic` (Cortex-M3 générique) |
| Outils / docs | `embOSView/` (Windows), UM01001, UM01039, `SYSVIEW_embOS.txt` | embOSView inutilisable sous Linux |

Symboles externes exigés par `libosT7VHLDP.a` (liste complète dans `embos-api.txt`) :
`OS_Idle`, `OS_Error`, `OS_COM_Send1`, `OS_JLINKMEM_BufferSize`, `__stack_start__`,
`__stack_end__`, et côté libc newlib `malloc/free/realloc/memcpy/memset/strlen/_impure_ptr`,
`__aeabi_(u)ldivmod`. `_impure_ptr` (newlib) et `malloc` ne sont tirés que si les objets
correspondants sont référencés (thread-safety newlib, `OS_HEAP_*`) : **à vérifier contre la libc
Lepton (`lib/libc`) à l'étape 3** (HYPOTHÈSE À VALIDER : aucun conflit si Lepton n'appelle pas
`OS_HEAP_*` / `OS_ThreadSafe`).

## 3. Variantes de bibliothèques

Convention (UM01039 §3.1) : `libosT<Arch><VFP><Endian><LibMode><Errata><TrustZone><PACBTI>.a`

| Champ | Valeurs |
|---|---|
| Arch | `6` Cortex-M0/M0+/M1 ; `7` Cortex-M3/M4/M7 ; `8BL` M23 ; `8ML` M33 ; `81ML` M55/M85 |
| VFP | (vide) sans FPU ; `V` FPU, ABI **softfp** ; `VH` FPU, ABI **hard** (ajoutées en V4.40) |
| Endian | `L` little ; `B` big |
| LibMode | `XR`, `R`, `S`, `SP`, `D`, `DP`, `DT` (→ `OS_LIBMODE_XR…DT`, à définir à l'identique à la compilation) |
| Errata | `_837070` : contournement de l'erratum **Cortex-M7 r0p0/r0p1 837070** (écriture de BASEPRI non immédiate) ; exige `USE_ERRATUM_837070=1` (UM01039 §4.4). Le contournement n'est plus appliqué par défaut depuis V5.10.2.0 ; ces libs l'appliquent. |
| TZ / PACBTI | ARMv8-M uniquement |

54 familles × 7 modes = 378 ; décodage complet : `$LEPTON_BUILD/etape-1/kal/library-variants.csv`.
Pas de variante VFP pour ARMv6-M (UM01039 §3.1).

### Correspondance variante ↔ flags GCC (pour `cmake/cpu/*.cmake`)

Attributs ELF lus par `readelf -A` sur chaque famille (source « ELF ») et options des projets BSP
(source « BSP »).

| Cœur Lepton | Bibliothèque | Flags GCC | Justification |
|---|---|---|---|
| M0 / M0+ | `libosT6L<mode>.a` | `-mthumb -mcpu=cortex-m0` (ou `cortex-m0plus`) `-mfloat-abi=soft` | ELF : `Tag_CPU_arch v6S-M`, Thumb-1 ; BSP `STM32F051_…` et `STM32G0B1_…` liés à `osT6L*` |
| M3 | `libosT7L<mode>.a` | `-mthumb -mcpu=cortex-m3 -mfloat-abi=soft` | ELF : `v7`, `7-M`, Thumb-2, pas de FP ; BSP `CMSIS/Generic`, `STM32F103_STM32_SK` (`-mcpu=cortex-m3`, `libosT7L*`) |
| M4 sans FPU utilisée | `libosT7L<mode>.a` | `-mthumb -mcpu=cortex-m4 -mfloat-abi=soft` | BSP `STM32L476_…` : `-mcpu=cortex-m4` + `libosT7L*` |
| M4F, ABI softfp | `libosT7VL<mode>.a` | `-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp` | ELF : `v7E-M`, `VFPv4-D16`, `HardFP_use: SP only`, pas de `Tag_ABI_VFP_args` ; BSP F429ZI Nucleo (`.emP`) exactement ces flags avec `libosT7VLDP/VLR` |
| **M4F, ABI hard** | **`libosT7VHL<mode>.a`** | `-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard` | ELF : idem + `Tag_ABI_VFP_args: VFP registers` ; UM01039 §8.5 (la lib doit correspondre à l'ABI) |
| M7 (r1p0+) FPU hard | `libosT7VHL<mode>.a` | `-mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard` | Pas de lib « M7 » dédiée : famille `7` = M3/M4/M7 (UM01039). Les BSP M7 du paquet utilisent `fpv4-sp-d16` (F756, H743) ou `fpv5-d16` softfp (F767). HYPOTHÈSE À VALIDER (étape 6) : lien de libs `VFPv4-D16 SP` avec du code `fpv5-d16` accepté et correct (embOS sauve S0–S31 = D0–D15). |
| M7 r0p0/r0p1 | `libosT7VHL<mode>_837070.a` | idem + `-DUSE_ERRATUM_837070=1` | UM01039 §4.4 ; BSP F756 (`libosT7VLDP_837070.a`, `USE_ERRATUM_837070=1`) |

**Choix pour M4F (NUCLEO-F439ZI, QEMU `mps2-an386`)** — deux options, décision à l'étape 2 :

1. **`libosT7VHLDP.a` / `libosT7VHLR.a`** (hard-float, cible de CLAUDE.md). Impose que le KAL
   gère la trame FPU étendue (`OS_REGS_BASE_FPU`, voir `embos-iar-vs-gcc.md` §3, écart E3).
2. `libosT7LSP.a` / `libosT7LDP.a` (sans FPU, `-mfloat-abi=soft`) : **équivalent exact de la
   configuration IAR actuelle** (`tauon_8.40.ewp`, configuration `tauon-kernel-cortex-m4-debug` :
   FPU = none, `OS_LIBMODE_SP`, lib IAR `os7m_tl__sp.a`). Utile comme premier palier QEMU.

Recommandation : palier 1 en option 2 pour isoler les écarts compilateur, puis option 1 (hard)
dès que le KAL gère la trame FPU. Mode de lib : `DP` en debug (vérifications `OS_Error`), `R`
ou `SP` en release (l'existant IAR utilise `SP`).

## 4. BSP le plus proche du STM32F439

Candidats ST Cortex-M4 : `STM32F429_STM32F429ZI_Nucleo`, `STM32F429_STM32F429I_Discovery`,
`STM32F429_STM3242I_SK`, `STM32F407_STM32F4_Discovery`, `STM32F407_STM3240G_Eval`,
`STM32F401_STM32F401C_Discovery`. **Retenu : `ST/STM32F429_STM32F429ZI_Nucleo`** (même carte
NUCLEO-144, même boîtier/brochage ZI, même mémoire que le F439ZI).

Contenu du BSP retenu :

| Fichier | Rôle | Axe |
|---|---|---|
| `DeviceSupport/startup_stm32f429xx.S` (ST, GNU) | table des vecteurs, `Reset_Handler` → `SystemInit` → `__libc_init_array` → `main` | carte (ISA pour la partie générique) |
| `DeviceSupport/system_stm32f4xx.c/.h`, `stm32f4xx.h` (ST SPL 2013, `STM32F429_439xx`) | horloges : HSE 8 MHz, PLL M=8 N=336 P=2 Q=7 → **168 MHz** ; active CP10/CP11 si `__FPU_USED` | carte / cœur (FPU) |
| `Setup/STM32F429ZI_FLASH.ld` | FLASH 2048K @0x08000000, RAM **112K** @0x20000000, CCMRAM 64K @0x10000000, `__stack_start__/__stack_end__` | carte |
| `Setup/RTOSInit_STM32F4xx.c` | `OS_InitHW` (SysTick à `SystemCoreClock`/1000 Hz, `OS_SYSTIMER_CONFIG`), `SysTick_Handler` → `OS_TICK_Handle`, `OS_Idle`, `OS_COM_Send1` (J-Link par défaut, UART via `BSP_UART` optionnelle) | micro-noyau × carte |
| `Setup/BSP.c` | LEDs PB0/PB7/PB14 | carte |
| `Setup/OS_Error.c`, `OS_Syscalls.c`, `OS_ThreadSafe.c` | handler d'erreur debug, syscalls newlib, verrous newlib | micro-noyau |
| `Setup/HardFaultHandler.S` + `SEGGER_HardFaultHandler.c` | HardFault avancé | ISA |
| `CoreSupport/` | CMSIS 5 (`core_cm4.h`, `cmsis_gcc.h`, `mpu_armv7.h`) | cœur |
| `SEGGER/` | RTT, SystemView | outils (optionnel) |
| `Application/OS_*.c` | 20 exemples | non utilisés |
| Projet | `Start_STM32F429.emP` (+ `.cproject` SW4STM32) : `-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp`, `-DSTM32F429_439xx=1 -DHSE_VALUE=8000000`, `libosT7VLDP.a` (Debug) / `libosT7VLR.a` (Release) | |

Absent du BSP : pilote UART (`BSP_UART.c` n'existe que pour F407 Eval et L152 SK), Ethernet,
MPU/cache. Lepton garde ses propres pilotes (`dev/arch/cortexm/stm32f4xx`).

Écarts F429 ↔ F439 et points à valider :

- Le F439 = F429 + accélérateur **CRYP** (AES/DES/TDES) et **HASH** (connaissance datasheet ST,
  DS9484/RM0090 ; HYPOTHÈSE À VALIDER sur le RM0090). Vérifié dans le paquet :
  `startup_stm32f429xx.S` met **0 (réservé) à la place de `CRYP_IRQHandler`** (IRQ 79, entre
  DCMI et HASH_RNG) ; `stm32f4xx.h` déclare CRYP/HASH pour `STM32F429_439xx`. Lepton possède déjà
  `startup_stm32f439xx.s` IAR (avec `CRYP_IRQHandler`) : pour le F439, prendre une startup GCC
  F439 (CMSIS ST) ou corriger ce vecteur.
- RAM : le `.ld` n'exploite que SRAM1 (112K) ; le F429/439 a 192K contiguës (SRAM1+2+3) + 64K CCM (HYPOTHÈSE À VALIDER sur RM0090).
  Lepton IAR (`tauon-basic_stm32f407-discovery/FLASH.icf`) utilise 112K + CCM. À trancher à
  l'étape 5 (`ld/mem_nucleo-f439zi.ld`).
- Horloge : `SetSysClock` met `HSEON` **sans `HSEBYP`** alors que le HSE de la NUCLEO-144 est
  l'horloge MCO 8 MHz du ST-LINK (signal externe). HYPOTHÈSE À VALIDER sur carte : bypass
  nécessaire ; sinon repli silencieux sur HSI 16 MHz et `SystemCoreClock` erroné pour SysTick.
  168 MHz (et non 180 MHz max du F439).
- QEMU `mps2-an386` : aucun BSP ; base proposée `CMSIS/Generic` (Cortex-M3 générique,
  `startup.s`, `Generic_CortexM_flash.ld` ROM 128K @0 / RAM 64K @0x20000000, `RTOSInit_CMSIS.c`
  SysTick générique, `libosT7L*`) — adresses à adapter à l'AN386.

## 5. Licence

`License.txt` : **SEGGER's Friendly License (SFL), 4 nov. 2022**.

- Gratuit pour **usage non commercial ou évaluation** ; tout autre usage = licence commerciale
  SEGGER. « Évaluation » cesse quand l'usage devient partie standard du flux de travail.
- Interdits : décompiler/désassembler, **redistribuer**, sous-licencier ; retirer les mentions.
- Le code source marqué ne peut servir qu'avec le logiciel, et ses objets exigent une licence
  valide ; SEGGER peut révoquer au cas par cas ; droit allemand.
- Décision utilisateur 2026-09-30 : usage évaluation/non commercial ; paquet hors git sous
  `~/lepton/third_party/embos/cortexm-gcc/5.20.0.0`, désigné par `LEPTON_EMBOS_ROOT`
  (`scripts/lepton-env.sh`).

Conséquences pour la suite :

- Aucun fichier du paquet (lib, `RTOS.h`, `RTOSInit`, `.ld`, startup) ne doit entrer dans git ;
  CMake les référence via `LEPTON_EMBOS_ROOT`. La CI (`ci/Dockerfile`) doit monter le paquet,
  jamais l'embarquer dans l'image publiée.
- Les fichiers Lepton dérivés d'exemples Segger (`RTOSInit`, `main.c`) : **point de décision** —
  soit compilés depuis `LEPTON_EMBOS_ROOT` (copie exclue de git), soit réécrits par Lepton.
- Constat : le dépôt contient déjà du code Segger versionné (`kernel/core/ucore/embOS*`, dont
  `embOSCXM4_518` : RTOS.h 5.18.3.1 IAR + `os7m_tl__sp.a`). Situation héritée, signalée, non
  traitée ici.
- `embOSView/` contient des DLL Windows tierces (`3rd-party.txt`) : sans usage sous Linux.

## 6. API publique

`doc/migration/embos-api.txt` (généré) : 334 fonctions (marquées exportées ou non par
`libosT7VHLDP.a`), 766 macros (alias de compatibilité signalés `->`), 85 types, 158 constantes
d'énumération, champs de `OS_TASK_STRUCT`, `OS_REGS_BASE(_FPU)`, `OS_GLOBAL_STRUCT`, et les
66 symboles exportés non déclarés (internes, dont `OS_MakeTaskReady`, `PendSV_Handler`).

Régénération :

```bash
source scripts/lepton-env.sh
python3 tools/migration/embos_inventory.py --embos "$LEPTON_EMBOS_ROOT" \
  --ref "$LEPTON_TRUNK/sys/root/src/kernel/core/ucore/embOSCXM4_518/inc/RTOS.h" \
  --trunk "$LEPTON_TRUNK" --api doc/migration/embos-api.txt --work "$LEPTON_BUILD/etape-1/kal"
```
