# Inventaire des projets IAR (étape 1, tâche 1)

Généré par `tools/migration/ewp_extract.py` le 2026-09-30 (trunk `trunk`). Ne pas éditer à la main : rejouer `python3 tools/migration/ewp_extract.py --markdown doc/migration/inventaire-projets.md` depuis la racine du clone. JSON détaillé par projet : `$LEPTON_BUILD/etape-1/projets/`.

## Synthèse

- `.ewp` : **46** (fileVersion : v1 = 3, v2 = 33, v3 = 6, v4 = 4) ; `.eww` : **18** ; `.ewd` : **0** (aucun fichier de configuration du débogueur dans l'arbre).
- Étapes custom (pre/post-build, CUSTOM, buildActions) : **0** configuration(s) non vide(s) ; projets citant `mklepton` : **0**. La génération mklepton (`kernel_mkconf.h`, `dev_mkconf.c`, `bin_mkconf.c`, `dev_dskimg.[ch]` sous `src/kernel/core/arch/<arch>/`) est donc lancée hors des projets IAR ; ces fichiers sont référencés mais absents de l'arbre (voir « Sources absentes »).
- Tous les chemins absolus des projets sont en `c:\tauon\…` (ou `C:/tauon/…`) : `tauon_make_link.bat` crée la jonction `c:\tauon` → racine de l'arbre ; ils sont ramenés au trunk.
- Cœur par configuration : provenance indiquée (`xcl` = option `--cpu` du `settings/*.driver.xcl` généré par EWARM ; `xcl-corrélé` = même code CoreVariant/Variant qu'une config. dont le `.xcl` existe ; `nom-config` = déduit du nom de configuration ; `puce` = famille de la puce sélectionnée).

Table apprise des `.driver.xcl` (code IAR → cœur / FPU), seule base des provenances `xcl-corrélé` :

| Code IAR | Cœur(s) observé(s) (nb de .xcl) |
|---|---|
| CoreVariant=35 | Cortex-M0+ (1) |
| CoreVariant=38 | Cortex-M3 (2) |
| CoreVariant=39 | Cortex-M4 (3) |
| CoreVariant=41 | Cortex-M7 (3) |
| FPU2=0 | None (6) |
| FPU2=6 | VFPv5_SP (3) |

Table apprise des noms de configuration (provenance `nom-config-corrélé`) :

| Code IAR | Cœur(s) d'après le nom de configuration (nb de config.) |
|---|---|
| CoreVariant=12 | ARM926EJ-S (8) |
| CoreVariant=35 | Cortex-M0+ (4) |
| CoreVariant=38 | Cortex-M3 (4) |
| CoreVariant=39 | Cortex-M4 (10) |
| CoreVariant=41 | Cortex-M7 (9) |
| Variant=12 | ARM926EJ-S (2) |
| Variant=37 | Cortex-M3 (1) |
| Variant=38 | Cortex-M4 (1) |
| Variant=39 | Cortex-M4 (1) |

HYPOTHÈSE À VALIDER : les codes CoreVariant/Variant non couverts par un `.xcl` (ex. `Variant` des projets EWARM 4.x–6.x, `FPU2=4`, `FPU2=7`) sont interprétés par le nom de configuration ou la puce ; aucune table officielle IAR n'est disponible dans l'arbre.

### Tableau synthétique

Colonnes : fV = fileVersion ; EW = version EWARM du dernier enregistrement (max. des configurations) ; src = nombre max. de sources C/asm d'une configuration ; abs. = fichiers référencés absents (hors bibliothèques de sortie).

| Projet (sous `sys/`) | fV | EW | Carte / famille | Configurations : cœur, sortie | src | abs. | Rôle proposé |
|---|---|---|---|---|---|---|---|
| `root/prj/iar/arch/arm/dev-nxp-nfc-pn7150/dev_nxp_nfc_pn7150.ewp` | 2 | 7.80.4.12487 | NXP PN7150 (NFC) | cortex-m4-debug : Cortex-M4, lib<br>cortex-m7-debug : Cortex-M7 VFPv5_SP, lib | 1 | 0 | périphérique optionnel |
| `root/prj/iar/arch/arm/dev/at91m55800a/dev_at91m55800a.ewp` | 1 | 4.41A | AT91M55800 (ARM7) | lib-debug : ARM7TDMI, lib | 5 | 0 | gelé (ARM7/ARM9) |
| `root/prj/iar/arch/arm/dev/at91sam7x/dev_at91sam7x.ewp` | 1 | 4.41A | AT91SAM7X (ARM7) | lib-debug : ARM7TDMI, lib | 12 | 21 | gelé (ARM7/ARM9) |
| `root/prj/iar/arch/arm/dev/at91sam9261/dev_at91sam9261.ewp` | 2 | 6.21.1.52845 | AT91SAM9261 (ARM9) | lib-debug : ARM926EJ-S, lib | 14 | 0 | gelé (ARM7/ARM9) |
| `root/prj/iar/arch/arm/dev/at91sam9261/dev_at91sam9261_7.20.ewp` | 2 | 7.20.1.7306 | AT91SAM9261 (ARM9) | lib-debug : ARM926EJ-S, lib | 14 | 0 | gelé (ARM7/ARM9) |
| `root/prj/iar/arch/arm/dev/at91samd20/dev_at91samd20_7.20.ewp` | 2 | 7.40.3.8937 | SAMD20 Xplained Pro | Debug : Cortex-M0+, lib<br>Release : ?, exe | 12 | 1 | candidat étape 6 (M0+/M3) |
| `root/prj/iar/arch/arm/dev/at91samv7x/dev_at91samv7x_7.80.ewp` | 2 | 7.80.4.12487 | SAMV71 Xplained Ultra | Debug : Cortex-M3, lib<br>Release : Cortex-M3, exe<br>samv71-freertos-debug : Cortex-M7 VFPv5_SP, lib | 57 | 0 | M7 Atmel (carte non retenue) |
| `root/prj/iar/arch/arm/dev/lm3s/dev_lm3s_6.21.ewp` | 2 | 6.21.1.52845 | Stellaris LM3S | Debug : Cortex-M3, lib | 33 | 0 | candidat étape 6 (M0+/M3) |
| `root/prj/iar/arch/arm/dev/stm32f1xx/dev_stm32f1xx_6.21.ewp` | 2 | 6.21.1.52845 | STM32F1 (famille) | Debug : Cortex-M3, lib | 36 | 0 | candidat étape 6 (M0+/M3) |
| `root/prj/iar/arch/arm/dev/stm32f4xx/dev_stm32f4xx_6.21.ewp` **[F4]** | 2 | 6.21.1.52845 | STM32F4 (famille) | Debug : Cortex-M4, lib | 43 | 7 | base F439 (STM32F4) |
| `root/prj/iar/arch/arm/dev/stm32f4xx/dev_stm32f4xx_7.20.ewp` **[F4]** | 2 | 7.80.4.12487 | STM32F4 (famille) | Debug : Cortex-M4, lib | 107 | 2 | base F439 (STM32F4) |
| `root/prj/iar/arch/arm/dev/stm32f4xx/dev_stm32f4xx_8.40.ewp` **[F4]** | 3 | 8.40.1.21529 | STM32F4 (famille) | Debug : Cortex-M4, lib | 107 | 2 | base F439 (STM32F4) |
| `root/prj/iar/arch/arm/dev/stm32wlxx/dev_stm32wlxx_8.40.ewp` | 3 | 8.40.1.21529 | STM32WL55 Nucleo | Debug : Cortex-M4, lib<br>Release : Cortex-M3, exe | 63 | 0 | STM32WL (carte non listée) |
| `root/prj/iar/arch/arm/dev/stm32wlxx/dev_stm32wlxx_9.50.ewp` | 4 | 9.50.2.71646 | STM32WL55 Nucleo | Debug : Cortex-M4, lib<br>Release : Cortex-M3, exe | 63 | 0 | STM32WL (carte non listée) |
| `root/prj/iar/arch/arm/stm32f4-usb-core/Backup of stm32f4_usb_core.ewp` **[F4]** | 2 | 7.40.3.8937 | STM32F4 (USB device) | Debug : Cortex-M4, lib<br>Release : ?, exe | 12 | 1 | STM32F4, USB device (optionnel) |
| `root/prj/iar/arch/arm/stm32f4-usb-core/stm32f4_usb_core.ewp` **[F4]** | 2 | 7.80.4.12487 | STM32F4 (USB device) | Debug : Cortex-M4, lib<br>Release : ?, exe | 19 | 1 | STM32F4, USB device (optionnel) |
| `root/prj/iar/arch/arm/stm32f4-usb-core/stm32f4_usb_core_8.40.ewp` **[F4]** | 3 | 8.40.1.21529 | STM32F4 (USB device) | Debug : Cortex-M4, lib<br>Release : ?, exe | 19 | 1 | STM32F4, USB device (optionnel) |
| `root/prj/iar/arch/arm/tauon/tauon.ewp` | 1 | 4.42A | noyau Lepton (multi-cœur) | lib-debug : ARM926EJ-S, lib | 284 | 254 | noyau (config. cortex-m4 → F439) |
| `root/prj/iar/arch/arm/tauon/tauon_6.10.ewp` | 2 | 6.10.3.52260 | noyau Lepton (multi-cœur) | lib-debug : Cortex-M3, lib | 172 | 256 | noyau (config. cortex-m4 → F439) |
| `root/prj/iar/arch/arm/tauon/tauon_6.21.ewp` | 2 | 6.21.1.52845 | noyau Lepton (multi-cœur) | lib-debug : Cortex-M3, lib<br>tauon-kernel-cortex-m4-debug : Cortex-M4, lib<br>tauon-kernel-arm926ejs-debug : ARM926EJ-S, lib<br>tauon-kernel-cortex-m3-debug : Cortex-M3, lib<br>tauon-kernel-cortex-m4f-freertos-debug : Cortex-M4 FPU=5, lib<br>tauon-kernel-arm926ejs-freertos-debug : ARM926EJ-S, lib | 212 | 259 | noyau (config. cortex-m4 → F439) |
| `root/prj/iar/arch/arm/tauon/tauon_7.20.ewp` | 2 | 7.80.4.12487 | noyau Lepton (multi-cœur) | lib-debug : Cortex-M3, lib<br>tauon-kernel-cortex-m4-debug : Cortex-M4, lib<br>tauon-kernel-arm926ejs-debug : ARM926EJ-S, lib<br>tauon-kernel-cortex-m3-debug : Cortex-M3, lib<br>tauon-kernel-cortex-m4f-freertos-debug : Cortex-M4 FPU2=4, lib<br>tauon-kernel-arm926ejs-freertos-debug : ARM926EJ-S, lib<br>tauon-kernel-cortex-m0+-freertos-debug : Cortex-M0+, lib | 225 | 257 | noyau (config. cortex-m4 → F439) |
| `root/prj/iar/arch/arm/tauon/tauon_7.80.ewp` | 2 | 7.80.4.12487 | noyau Lepton (multi-cœur) | lib-debug : Cortex-M3, lib<br>tauon-kernel-cortex-m4-debug : Cortex-M4, lib<br>tauon-kernel-arm926ejs-debug : ARM926EJ-S, lib<br>tauon-kernel-cortex-m3-debug : Cortex-M3, lib<br>tauon-kernel-cortex-m4f-freertos-debug : Cortex-M4 FPU2=4, lib<br>tauon-kernel-arm926ejs-freertos-debug : ARM926EJ-S, lib<br>tauon-kernel-cortex-m0+-freertos-debug : Cortex-M0+, lib<br>tauon-kernel-cortex-m4m7-freertosv9-debug : Cortex-M7 VFPv5_SP, lib<br>tauon-kernel-cortex-m7-debug : Cortex-M7 VFPv5_SP, lib | 247 | 244 | noyau (config. cortex-m4 → F439) |
| `root/prj/iar/arch/arm/tauon/tauon_8.40.ewp` | 3 | 8.40.1.21529 | noyau Lepton (multi-cœur) | lib-debug : Cortex-M3, lib<br>tauon-kernel-cortex-m4-debug : Cortex-M4, lib<br>tauon-kernel-arm926ejs-debug : ARM926EJ-S, lib<br>tauon-kernel-cortex-m3-debug : Cortex-M3, lib<br>tauon-kernel-cortex-m4f-freertos-debug : Cortex-M4 FPU2=4, lib<br>tauon-kernel-arm926ejs-freertos-debug : ARM926EJ-S, lib<br>tauon-kernel-cortex-m0+-freertos-debug : Cortex-M0+, lib<br>tauon-kernel-cortex-m4m7-freertosv9-debug : Cortex-M7 VFPv5_SP, lib<br>tauon-kernel-cortex-m7-debug : Cortex-M7 VFPv5_SP, lib | 248 | 244 | noyau (config. cortex-m4 → F439) |
| `root/prj/iar/arch/arm/tauon/tauon_9.50.ewp` | 4 | 9.50.2.71646 | noyau Lepton (multi-cœur) | lib-debug : Cortex-M3, lib<br>tauon-kernel-cortex-m4-debug : Cortex-M4, lib<br>tauon-kernel-arm926ejs-debug : ARM926EJ-S, lib<br>tauon-kernel-cortex-m3-debug : Cortex-M3, lib<br>tauon-kernel-cortex-m4f-freertos-debug : Cortex-M4 FPU2=4, lib<br>tauon-kernel-arm926ejs-freertos-debug : ARM926EJ-S, lib<br>tauon-kernel-cortex-m0+-freertos-debug : Cortex-M0+, lib<br>tauon-kernel-cortex-m4m7-freertosv9-debug : Cortex-M7 VFPv5_SP, lib<br>tauon-kernel-cortex-m7-debug : Cortex-M7 VFPv5_SP, lib | 247 | 244 | noyau (config. cortex-m4 → F439) |
| `root/prj/iar/bsp/discovery_f4-baseboard-modem/bsp_discovery_f4-baseboard-modem_7.40.ewp` **[F4]** | 2 | 7.80.4.12487 | STM32F4 Discovery + modem | Debug : Cortex-M4, lib<br>Release : ?, exe | 14 | 0 | variante STM32F4 non retenue |
| `root/prj/iar/bsp/discovery_f4/bsp_discovery_f4_7.30.ewp` **[F4]** | 2 | 7.40.3.8937 | STM32F4-Discovery | Debug : Cortex-M4, lib<br>Release : ?, exe | 2 | 0 | base F439 (STM32F4) |
| `root/prj/iar/bsp/olimex_p407/bsp_olimex_p407_7.30.ewp` **[F4]** | 2 | 7.80.4.12487 | Olimex STM32-P407 | Debug : Cortex-M4, lib<br>Release : ?, exe | 3 | 0 | base F439 (STM32F4) |
| `root/prj/iar/bsp/samd20xplained_pro/bsp_samd20xplained_pro_7.30.ewp` | 2 | 7.40.3.8937 | SAMD20 Xplained Pro | Debug : Cortex-M0+, lib<br>Release : ?, exe | 4 | 0 | candidat étape 6 (M0+/M3) |
| `root/prj/iar/bsp/same70xplained/bsp_same70xplained_7.80.ewp` | 2 | 7.80.4.12487 | SAME70 Xplained | Debug : Cortex-M3, lib<br>Release : Cortex-M3, exe<br>freertos-debug : Cortex-M7 VFPv5_SP, lib | 4 | 0 | M7 Atmel (carte non retenue) |
| `root/prj/iar/bsp/samv71xplained_ultra/bsp_samv71xplained_ultra_7.80.ewp` | 2 | 7.80.4.12487 | SAMV71 Xplained Ultra | Debug : Cortex-M3, lib<br>Release : Cortex-M3, exe<br>freertos-debug : Cortex-M7 VFPv5_SP, lib | 6 | 0 | M7 Atmel (carte non retenue) |
| `root/prj/iar/bsp/stm32f469i-eval/bsp-stm32f469i-eval.ewp` **[F4]** | 2 | 7.80.4.12487 | STM32F469I-EVAL | Debug : Cortex-M4, lib<br>Release : ?, exe | 5 | 0 | base F439 (STM32F4) |
| `root/prj/iar/bsp/stm32wl55jci_nucleo/bsp_stm32wl55jci_nucleo_8.40.ewp` | 3 | 8.40.1.21529 | STM32WL55 Nucleo | Debug : Cortex-M4, lib<br>Release : Cortex-M3, exe | 4 | 0 | STM32WL (carte non listée) |
| `root/prj/iar/bsp/stm32wl55jci_nucleo/bsp_stm32wl55jci_nucleo_9.50.ewp` | 4 | 9.50.2.71646 | STM32WL55 Nucleo | Debug : Cortex-M4, lib<br>Release : Cortex-M3, exe | 4 | 0 | STM32WL (carte non listée) |
| `root/prj/iar/lib/lib-nxpnfc/lib-nxpnfc.ewp` | 2 | 7.80.4.12487 | NXP PN7150 (NFC) | cortex-m4-debug : Cortex-M4, lib<br>cortex-m7-debug : Cortex-M7 VFPv5_SP, lib | 11 | 0 | périphérique optionnel |
| `root/src/kernel/dev/arch/cortexm/stellaris/driverlib/driverlib.ewp` | 2 | 6.21.1.52845 | Stellaris LM3S | Debug : Cortex-M3, lib | 23 | 0 | candidat étape 6 (M0+/M3) |
| `user/tauon-basic/prj/iar/arch/arm/tauon-basic-stm32wl55jci-nucleo/tauon-basic_stm32wl55jci_nucleo_8.40.ewp` | 3 | 8.40.1.21529 | STM32WL55 Nucleo | Debug : Cortex-M4, exe<br>Release : Cortex-M3, exe<br>freertos-debug : Cortex-M3, exe | 43 | 10 | STM32WL (carte non listée) |
| `user/tauon-basic/prj/iar/arch/arm/tauon-basic-stm32wl55jci-nucleo/tauon-basic_stm32wl55jci_nucleo_9.50.ewp` | 4 | 9.50.2.71646 | STM32WL55 Nucleo | Debug : Cortex-M4, exe<br>Release : Cortex-M3, exe<br>freertos-debug : Cortex-M3, exe | 43 | 10 | STM32WL (carte non listée) |
| `user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91sam9261/tauon_at91sam9261.ewp` | 2 | 6.21.1.52845 | AT91SAM9261 (ARM9) | firmware : ARM7TDMI, exe<br>firmware_injector : ARM926EJ-S, exe<br>firmware_flash : ARM7TDMI, exe<br>freertos-firmware_injector : ARM926EJ-S, exe | 37 | 22 | gelé (ARM7/ARM9) |
| `user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91sam9261/tauon_at91sam9261_7.20.ewp` | 2 | 7.20.1.7306 | AT91SAM9261 (ARM9) | firmware : ARM7TDMI, exe<br>firmware_injector : ARM926EJ-S, exe<br>firmware_flash : ARM7TDMI, exe<br>freertos-firmware_injector : ARM926EJ-S, exe | 37 | 22 | gelé (ARM7/ARM9) |
| `user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91samd20/tauon-basic_at91samd20_7.20.ewp` | 2 | 7.40.3.8937 | SAMD20 Xplained Pro | Debug : Cortex-M0+, exe<br>Release : ?, exe<br>freertos-debug : Cortex-M0+, exe | 7 | 5 | candidat étape 6 (M0+/M3) |
| `user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91samv71/tauon-basic_at91samv71_7.80.ewp` | 2 | 7.80.4.12487 | SAMV71 Xplained Ultra | Debug : Cortex-M7 FPU2=7, exe<br>Release : ?, exe<br>freertos-debug : Cortex-M7 VFPv5_SP, exe | 21 | 6 | M7 Atmel (carte non retenue) |
| `user/tauon-basic/prj/iar/arch/arm/tauon-basic_cmsis/tauon-basic_cmsis_6.21.ewp` | 2 | 6.21.1.52845 | générique Cortex-M (LM3S) | Debug : Cortex-M3, exe<br>Release : Cortex-M3, exe | 19 | 16 | candidat étape 6 (M0+/M3) |
| `user/tauon-basic/prj/iar/arch/arm/tauon-basic_lm3s/tauon-basic_lm3s_6.21.ewp` | 2 | 6.21.1.52845 | Stellaris LM3S | Debug : Cortex-M3, exe<br>Release : Cortex-M3, exe | 42 | 23 | candidat étape 6 (M0+/M3) |
| `user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f4-olimex_p407/tauon-basic_stm32f4-olimex_p407_7.20.ewp` **[F4]** | 2 | 7.80.4.12487 | Olimex STM32-P407 | Debug : Cortex-M4, exe<br>Release : Cortex-M3, exe<br>freertos-debug : Cortex-M3, exe | 38 | 10 | base F439 (STM32F4) |
| `user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f407-discovery/tauon-basic_stm32f407_discovery_7.20.ewp` **[F4]** | 2 | 7.80.4.12487 | STM32F4-Discovery | Debug : Cortex-M4, exe<br>Release : Cortex-M3, exe<br>freertos-debug : Cortex-M3, exe | 38 | 10 | base F439 (STM32F4) |
| `user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f469i-eval/tauon-basic_stm32f469i_eval_7.80.ewp` **[F4]** | 2 | 7.80.4.12487 | STM32F469I-EVAL | Debug : Cortex-M4, exe<br>Release : Cortex-M3, exe<br>freertos-debug : Cortex-M3, exe | 21 | 10 | base F439 (STM32F4) |

## Signalements

### Projets STM32F4 (base de la NUCLEO-F439ZI)

- `sys/root/prj/iar/arch/arm/dev/stm32f4xx/dev_stm32f4xx_6.21.ewp` — STM32F4 (famille)
- `sys/root/prj/iar/arch/arm/dev/stm32f4xx/dev_stm32f4xx_7.20.ewp` — STM32F4 (famille)
- `sys/root/prj/iar/arch/arm/dev/stm32f4xx/dev_stm32f4xx_8.40.ewp` — STM32F4 (famille)
- `sys/root/prj/iar/arch/arm/stm32f4-usb-core/Backup of stm32f4_usb_core.ewp` — STM32F4 (USB device)
- `sys/root/prj/iar/arch/arm/stm32f4-usb-core/stm32f4_usb_core.ewp` — STM32F4 (USB device)
- `sys/root/prj/iar/arch/arm/stm32f4-usb-core/stm32f4_usb_core_8.40.ewp` — STM32F4 (USB device)
- `sys/root/prj/iar/bsp/discovery_f4-baseboard-modem/bsp_discovery_f4-baseboard-modem_7.40.ewp` — STM32F4 Discovery + modem
- `sys/root/prj/iar/bsp/discovery_f4/bsp_discovery_f4_7.30.ewp` — STM32F4-Discovery
- `sys/root/prj/iar/bsp/olimex_p407/bsp_olimex_p407_7.30.ewp` — Olimex STM32-P407
- `sys/root/prj/iar/bsp/stm32f469i-eval/bsp-stm32f469i-eval.ewp` — STM32F469I-EVAL
- `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f4-olimex_p407/tauon-basic_stm32f4-olimex_p407_7.20.ewp` — Olimex STM32-P407
- `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f407-discovery/tauon-basic_stm32f407_discovery_7.20.ewp` — STM32F4-Discovery
- `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f469i-eval/tauon-basic_stm32f469i_eval_7.80.ewp` — STM32F469I-EVAL

Ces projets s'appuient sur le noyau `prj/iar/arch/arm/tauon/tauon_*.ewp`, configuration `tauon-kernel-cortex-m4-debug` (bibliothèque `building/output/tauon_7.80/tauon-kernel-cortex-m4-debug/Lib/tauon_7.80.a` liée par les applications `tauon-basic_stm32f4*`/`stm32f469i`), et sur `dev_stm32f4xx_7.20` (config. `Debug`).

### Cartes de l'étape 6

- **Olimex STM32-P407** (M4F) : `prj/iar/bsp/olimex_p407/bsp_olimex_p407_7.30.ewp` et application `user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f4-olimex_p407/…_7.20.ewp` ; même famille que la F439.
- **Discovery F7** : **aucun projet STM32F7 dans l'arbre** (ni `.ewp`, ni répertoire `stm32f7*`). Le seul code Cortex-M7 existant est Atmel SAMV71/SAME70 (`dev/arch/cortexm/at91samv7x`, `bsp/samv71xplained_ultra`, `bsp/same70xplained`) et les configurations `tauon-kernel-cortex-m7-debug` / `…-m4m7-freertosv9-debug` du noyau.
- **M3** (à choisir) : `dev_stm32f1xx_6.21.ewp` (STM32F1), `dev_lm3s_6.21.ewp` + `driverlib.ewp` + `tauon-basic_lm3s_6.21.ewp` / `tauon-basic_cmsis_6.21.ewp` (Stellaris LM3S) ; noyau `tauon-kernel-cortex-m3-debug`.
- **M0/M0+** (à choisir) : `dev_at91samd20_7.20.ewp`, `bsp_samd20xplained_pro_7.30.ewp`, `tauon-basic_at91samd20_7.20.ewp` (SAMD20) ; noyau `tauon-kernel-cortex-m0+-freertos-debug` (FreeRTOS seulement).

### Sources et fichiers référencés absents de l'arbre

Regroupés par répertoire (7 niveaux) ; nombre de projets citant :

| Répertoire / fichier absent | Projets |
|---|---|
| `$PROJ_DIR$\..\..\..\..\..\..\..\..\..\lepton\rootstock-mainline\depots\lepton\root\master\sched\freertos\scion\sys\root\src\kernel\core\ucore\freeRTOS_9-0-0\source\arch\cortex-m7\at91samv71\main.c` | 1 |
| `sys/root/src/bin/tst/verifspi.c` | 2 |
| `sys/root/src/kernel/core/arch/arm` | 16 |
| `sys/root/src/kernel/core/arch/cortexm` | 14 |
| `sys/root/src/kernel/core/devio.c` | 3 |
| `sys/root/src/kernel/core/devio.h` | 3 |
| `sys/root/src/kernel/core/net/lwip_core` | 4 |
| `sys/root/src/kernel/core/net/uip_core` | 4 |
| `sys/root/src/kernel/core/ucore/embOSARM7_332` | 9 |
| `sys/root/src/kernel/core/ucore/embOSARM7_360` | 9 |
| `sys/root/src/kernel/core/ucore/embOSCXM3_382` | 2 |
| `sys/root/src/kernel/core/ucore/freeRTOS_8-0-0` | 5 |
| `sys/root/src/kernel/dev/arch/arm7` | 1 |
| `sys/root/src/kernel/dev/arch/at91` | 1 |
| `sys/root/src/kernel/dev/arch/cortexm` | 3 |
| `sys/root/src/kernel/dev/dev_null/dev_null.h` | 7 |
| `sys/root/src/kernel/net/lwip/core` | 4 |
| `sys/root/src/kernel/net/lwip/netif` | 4 |
| `sys/root/src/kernel/usb/stm32f4-usb-core/stm32_usb_device_library` | 3 |

Remarques : `embOSARM7_332`, `embOSARM7_360/src`, `embOSCXM3_380/382` sont des versions d'embOS IAR absentes (autres versions présentes) ; `core/arch/{arm,cortexm}/*_mkconf.*` et `dev_dskimg.*` sont des sorties de mklepton (cf. `src/kernel/core/arch/cortexm/mklepton-output-generation.md`).

Fichiers de projet hors trunk ou sur un autre poste :

- `$PROJ_DIR$\..\..\..\..\..\..\..\..\..\lepton\rootstock-mainline\depots\lepton\root\master\sched\embos\scion\sys\root\src\kernel\core\ucore\embOSCXM7_430\Lib\os7m_tl__sp.a` (tauon-basic_at91samv71_7.80.ewp)
- `$PROJ_DIR$\..\..\..\..\..\..\..\..\..\lepton\rootstock-mainline\depots\lepton\root\master\sched\freertos\scion\sys\root\src\kernel\core\ucore\freeRTOS_9-0-0\source\arch\cortex-m7\at91samv71\main.c` (tauon-basic_at91samv71_7.80.ewp)

Chemins d'include non résolus (occurrences, toutes configurations) : absent = 208, x: = 24.

### Autres anomalies

- `prj/iar/arch/arm/stm32f4-usb-core/Backup of stm32f4_usb_core.ewp` : copie de sauvegarde EWARM (compte dans les 46 `.ewp`).
- Plusieurs générations du même projet coexistent (`tauon.ewp`, `tauon_6.10` … `tauon_9.50` ; `dev_stm32f4xx_6.21/7.20/8.40` ; `…_8.40`/`…_9.50`) ; les configurations `Release` des BSP/pilotes sont des squelettes (sans include ni define propre, cœur non renseigné).
- Puce sélectionnée incohérente avec le cœur (ex. `LM3S9D96` dans toutes les configurations du noyau, `STM32F207ZG` dans `freertos-debug` des applications STM32F4) : le cœur effectif est donné par CoreVariant lorsque OGCoreOrChip = 0.
- Surcharges d'options par fichier : `dev_stm32f4xx_7.20.ewp` (6 fichier(s)); `dev_stm32f4xx_8.40.ewp` (6 fichier(s)); `tauon_6.10.ewp` (1 fichier(s)); `tauon_6.21.ewp` (1 fichier(s)); `tauon_7.20.ewp` (4 fichier(s)); `tauon_7.80.ewp` (4 fichier(s)); `tauon_8.40.ewp` (4 fichier(s)); `tauon_9.50.ewp` (4 fichier(s)); `tauon-basic_lm3s_6.21.ewp` (1 fichier(s)); `tauon-basic_stm32f4-olimex_p407_7.20.ewp` (1 fichier(s)); `tauon-basic_stm32f407_discovery_7.20.ewp` (1 fichier(s)).

## Détail par projet

### `sys/root/prj/iar/arch/arm/dev-nxp-nfc-pn7150/dev_nxp_nfc_pn7150.ewp`

fileVersion 2 ; carte : NXP PN7150 (NFC) ; workspaces : aucun ; fichiers déclarés : 2 ; absents : 0.

- Includes communs : `src/kernel/core/ucore/cmsis/CMSIS/Include`, `src/kernel/dev/arch/all/i2c/nxp-nfc/NfcLibrary/NdefLibrary/inc (absent)`, `src/kernel/dev/arch/all/i2c/nxp-nfc/NfcLibrary/NxpNci/inc (absent)`, `src/kernel/dev/arch/all/i2c/nxp-nfc/NfcLibrary/inc (absent)`, `src/kernel/dev/arch/all/i2c/nxp-nfc/tml (absent)`, `src/kernel/dev/arch/all/i2c/nxp-nfc/tool (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `sys/root/src`
- **cortex-m4-debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.80.4.12487 ; sources c/cpp/asm 1/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\dev_nxp_nfc_pn7150.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `RW_SUPPORT`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`
- **cortex-m7-debug** — Cortex-M7 (xcl-corrélé), FPU VFPv5_SP (xcl-corrélé) ; puce `Default \| None` ; EW 7.80.4.12487 ; sources c/cpp/asm 1/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\dev_nxp_nfc_pn7150.a`
  - defines : `_SAMV71Q21__` `BOARD_SAMV71_XULT` `MPU_HAS_NOCACHE_REGION` `sram` `TRACE_LEVEL=0` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `RW_SUPPORT`
  - includes : `src/kernel/core/ucore/embOSCXM7_430/Inc`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/portable/IAR/ARM_CM7/r0p1`

### `sys/root/prj/iar/arch/arm/dev/at91m55800a/dev_at91m55800a.ewp`

fileVersion 1 ; carte : AT91M55800 (ARM7) ; workspaces : `dev_at91m55800a.eww` ; fichiers déclarés : 8 ; absents : 0.

- **lib-debug** — ARM7TDMI (puce), FPU None (FPU=0) ; puce `AT91M55800 \| Atmel AT91M55800` ; EW 4.41A ; sources c/cpp/asm 5/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\dev\at91m55800a\lib-debug\lib\dev_at91m55800a.r79`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP`
  - includes : `src/kernel/core/ucore/embOSARM7_332 (absent)`, `src/kernel/core/ucore/embOSARM7_332/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_332/src/generic (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`

### `sys/root/prj/iar/arch/arm/dev/at91sam7x/dev_at91sam7x.ewp`

fileVersion 1 ; carte : AT91SAM7X (ARM7) ; workspaces : `dev_at91sam7x.eww` ; fichiers déclarés : 21 ; absents : 21.

- **lib-debug** — ARM7TDMI (puce), FPU None (FPU=0) ; puce `AT91SAM7X256 \| Atmel AT91SAM7X256` ; EW 4.41A ; sources c/cpp/asm 12/0/0, exclus 1, absents 12 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\dev\at91sam7x\lib-debug\lib\dev_at91sam7x.r79`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP`
  - includes : `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`

### `sys/root/prj/iar/arch/arm/dev/at91sam9261/dev_at91sam9261.ewp`

fileVersion 2 ; carte : AT91SAM9261 (ARM9) ; workspaces : `dev_at91sam9261.eww`, `tauon_at91sam9261.eww` ; fichiers déclarés : 25 ; absents : 0.

- **lib-debug** — ARM926EJ-S (puce), FPU None (FPU=0) ; puce `AT91SAM9261 \| Atmel AT91SAM9261` ; EW 6.21.1.52845 ; sources c/cpp/asm 14/0/0, exclus 1, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\dev\at91sam9261\lib-debug\lib\dev_at91sam9261.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `SDRAM_TARGET=1` `at91sam9261`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `src/kernel/core/ucore/embOSARM7-9_388/inc`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`

### `sys/root/prj/iar/arch/arm/dev/at91sam9261/dev_at91sam9261_7.20.ewp`

fileVersion 2 ; carte : AT91SAM9261 (ARM9) ; workspaces : `tauon_at91sam9261_7.20.eww` ; fichiers déclarés : 25 ; absents : 0.

- **lib-debug** — ARM926EJ-S (puce), FPU None (FPU=0) ; puce `AT91SAM9261 \| Atmel AT91SAM9261` ; EW 7.20.1.7306 ; sources c/cpp/asm 14/0/0, exclus 1, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\dev\at91sam9261\lib-debug\lib\dev_at91sam9261_7.20.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `SDRAM_TARGET=1` `at91sam9261`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `src/kernel/core/ucore/embOSARM7-9_388/inc`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`

### `sys/root/prj/iar/arch/arm/dev/at91samd20/dev_at91samd20_7.20.ewp`

fileVersion 2 ; carte : SAMD20 Xplained Pro ; workspaces : `tauon-basic_at91samd20_7.20.eww` ; fichiers déclarés : 25 ; absents : 1.

- **Debug** — Cortex-M0+ (xcl-corrélé), FPU None (xcl-corrélé) ; puce `default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 11/0/0, exclus 3, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\dev\at91samd20\Debug\Exe\dev_at91samd20_7.20.a`
  - defines : `ARM_MATH_CM0=true` `_BOARD=SAMD20_XPLAINED_PRO` `EVENTS_INTERRUPT_HOOKS_MODE=true` `TC_ASYNC=true` `USART_CALLBACK_MODE=true` `__SAMD20J18__`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM0`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include`, `src/kernel/dev/arch/at91/asf/sam0/utils`, `src/kernel/dev/arch/at91/asf/sam0/utils/preprocessor`, `src/kernel/dev/arch/at91/asf/sam0/boards (absent)`, `src/kernel/dev/arch/at91/asf/sam0/boards/samd20_xplained_pro (absent)`, `src/kernel/dev/arch/at91/asf/sam0/drivers/port`, `src/kernel/dev/arch/at91/asf/sam0/drivers/sercom`, `src/kernel/dev/arch/at91/asf/sam0/drivers/tc`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/interrupt`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/interrupt/system_interrupt_samd20`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/clock`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/clock/clock_samd20`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/pinmux`, `src/kernel/dev/arch/at91/asf/sam0/drivers/events`, `src/kernel/dev/arch/at91/asf/common/utils`, `src/kernel/dev/arch/at91/asf/common/boards`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/source`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include/component`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include/pio`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include/instance`, `src/kernel/dev/arch/at91/asf/sam0/utils/header_files`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/clock/clock_samd20/module_config`
- **Release** — ? (indéterminé), FPU None (xcl-corrélé) ; puce `default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 12/0/0, exclus 0, absents 0 ; ICF par défaut EWARM (`$TOOLKIT_DIR$\CONFIG\generic.icf`)
  - defines : `NDEBUG`

### `sys/root/prj/iar/arch/arm/dev/at91samv7x/dev_at91samv7x_7.80.ewp`

fileVersion 2 ; carte : SAMV71 Xplained Ultra ; workspaces : `tauon-basic_at91samv71_7.80.eww` ; fichiers déclarés : 63 ; absents : 0.

- **Debug** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.80.4.12487 ; sources c/cpp/asm 57/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\dev_at91samv7x_7.80.a`
  - defines : `_SAMV71Q21__` `BOARD_SAMV71_XULT` `sram` `TRACE_LEVEL=4`
  - includes : `src/kernel/core/ucore/freeRTOS_9-0-0/source/arch/cortex-m0+/at91samd20`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/portable/IAR/ARM_CM0`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip/include`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip/include/samv71`, `src/kernel/dev/arch/at91/softpack-lib/samv71/samv71-xplained-ultra/libboard`, `src/kernel/dev/arch/at91/softpack-lib/samv71/samv71-xplained-ultra/libboard/include`
- **Release** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `-` ; EW 7.80.4.12487 ; sources c/cpp/asm 57/0/0, exclus 0, absents 0 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`
- **samv71-freertos-debug** — Cortex-M7 (puce), FPU VFPv5_SP (xcl-corrélé) ; puce `ATSAMV71Q21 \| Atmel ATSAMV71Q21` ; EW 7.80.4.12487 ; sources c/cpp/asm 54/0/0, exclus 3, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\dev_at91samv7x_7.80.a`
  - defines : `__SAMV71Q21__` `__LEPTON_SAME70_REVB__` `BOARD_SAMV71_XULT` `MPU_HAS_NOCACHE_REGION` `sram` `TRACE_LEVEL=0` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/embOSCXM7_430/Inc`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/portable/IAR/ARM_CM7/r0p1`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip/include`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip/include/samv71`, `src/kernel/dev/arch/at91/softpack-lib/samv71/samv71-xplained-ultra/libboard`, `src/kernel/dev/arch/at91/softpack-lib/samv71/samv71-xplained-ultra/libboard/include`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia/include`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia/include/nandflash`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia/include/sdmmc`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia/include/spiflash`

### `sys/root/prj/iar/arch/arm/dev/lm3s/dev_lm3s_6.21.ewp`

fileVersion 2 ; carte : Stellaris LM3S ; workspaces : `tauon-basic_lm3s_6.21.eww` ; fichiers déclarés : 34 ; absents : 0.

- **Debug** — Cortex-M3 (puce), FPU None (FPU=0) ; puce `LM3SxDxx \| TexasInstruments LM3SxDxx` ; EW 6.21.1.52845 ; sources c/cpp/asm 33/0/0, exclus 0, absents 0 ; bibliothèque → `$PROJ_DIR$\Debug\Exe\dev_lm3s.a`
  - defines : `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `TARGET_IS_TEMPEST_RB1` `PART_LM3S9D96`
  - defines asm : `ewarm`
  - includes : `src/kernel/dev/arch/cortexm/stellaris`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `sys/root/src`, `src/dev/arch/cortexm/stellaris/driverlib (absent)`

### `sys/root/prj/iar/arch/arm/dev/stm32f1xx/dev_stm32f1xx_6.21.ewp`

fileVersion 2 ; carte : STM32F1 (famille) ; workspaces : aucun ; fichiers déclarés : 70 ; absents : 0.

- **Debug** — Cortex-M3 (nom-config-corrélé), FPU None (FPU=0) ; puce `LM3SxDxx \| TexasInstruments LM3SxDxx` ; EW 6.21.1.52845 ; sources c/cpp/asm 36/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\dev\stm32f1xx\Debug\Exe\dev_stm32f1xx_6.21.a`
  - defines : `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `USE_STDPERIPH_DRIVER` `STM32F10X_XL`
  - defines asm : `ewarm`
  - includes : `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`

### `sys/root/prj/iar/arch/arm/dev/stm32f4xx/dev_stm32f4xx_6.21.ewp`

fileVersion 2 ; carte : STM32F4 (famille) ; workspaces : `tauon-basic_stm32f4_6.21.eww` ; fichiers déclarés : 86 ; absents : 7.

- **Debug** — Cortex-M4 (nom-config-corrélé), FPU None (FPU=0) ; puce `LM3SxDxx \| TexasInstruments LM3SxDxx` ; EW 6.21.1.52845 ; sources c/cpp/asm 43/0/0, exclus 1, absents 4 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\dev\stm32f4xx\Debug\Exe\dev_stm32f4xx_6.21.a`
  - defines : `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `USE_STDPERIPH_DRIVER`
  - defines asm : `ewarm`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`

### `sys/root/prj/iar/arch/arm/dev/stm32f4xx/dev_stm32f4xx_7.20.ewp`

fileVersion 2 ; carte : STM32F4 (famille) ; workspaces : `tauon-basic_stm32f4-olimex_p407_7.20.eww`, `tauon-basic_stm32f407_discovery_7.20.eww`, `tauon-basic_stm32f469i_eval_7.80.eww` ; fichiers déclarés : 160 ; absents : 2.

- **Debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.80.4.12487 ; sources c/cpp/asm 107/0/0, exclus 5, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\dev_stm32f4xx_7.20.a`
  - defines : `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `USE_STDPERIPH_DRIVER`
  - defines asm : `ewarm`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_440/Inc`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc`, `src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/legacy`

### `sys/root/prj/iar/arch/arm/dev/stm32f4xx/dev_stm32f4xx_8.40.ewp`

fileVersion 3 ; carte : STM32F4 (famille) ; workspaces : aucun ; fichiers déclarés : 160 ; absents : 2.

- **Debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 8.40.1.21529 ; sources c/cpp/asm 107/0/0, exclus 5, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\dev_stm32f4xx_8.40.a`
  - defines : `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `USE_STDPERIPH_DRIVER`
  - defines asm : `ewarm`
  - includes : `src/kernel/core/ucore/embOSCXM4_518/inc`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc`, `src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/legacy`

### `sys/root/prj/iar/arch/arm/dev/stm32wlxx/dev_stm32wlxx_8.40.ewp`

fileVersion 3 ; carte : STM32WL55 Nucleo ; workspaces : `tauon-basic_stm32wl55jci_nucleo_8.40.eww` ; fichiers déclarés : 80 ; absents : 0.

- **Debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 8.40.1.21529 ; sources c/cpp/asm 60/0/0, exclus 3, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\dev_stm32wlxx_8.40.a`
  - defines : `STM32WL55xx` `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `USE_STDPERIPH_DRIVER`
  - includes : `src/kernel/core/ucore/embOSCXM4_518/inc`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `src/kernel/core/ucore/cmsis/Device/st/stm32wlxx`, `src/kernel/dev/arch/cortexm/stm32wlxx/cubemx_hal_driver/inc`, `src/kernel/dev/arch/cortexm/stm32wlxx/dev_stm32wlxx`, `src/kernel/dev/arch/cortexm/stm32wlxx/radio_subghz_phy`, `src/kernel/dev/arch/cortexm/stm32wlxx/radio_subghz_phy/stm32_radio_driver`, `src/kernel/dev/arch/cortexm/stm32wlxx/radio_subghz_phy/stm32_radio_target`, `src/kernel/dev/arch/cortexm/stm32wlxx/radio_subghz_phy/stm32_radio_core_inc`, `src/kernel/dev/arch/cortexm/stm32wlxx/Utilities/misc`, `src/kernel/dev/arch/cortexm/stm32wlxx/Utilities/timer`, `src/kernel/dev/arch/cortexm/stm32wlxx/Utilities/trace/adv_trace`, `src/kernel/dev/arch/cortexm/stm32wlxx/Utilities/sequencer`
- **Release** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `-` ; EW 8.40.1.21529 ; sources c/cpp/asm 63/0/0, exclus 0, absents 0 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`

### `sys/root/prj/iar/arch/arm/dev/stm32wlxx/dev_stm32wlxx_9.50.ewp`

fileVersion 4 ; carte : STM32WL55 Nucleo ; workspaces : `tauon-basic_stm32wl55jci_nucleo_9.50.eww` ; fichiers déclarés : 80 ; absents : 0.

- **Debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 9.50.2.71646 ; sources c/cpp/asm 60/0/0, exclus 3, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Exe\dev_stm32wlxx_9.50.a`
  - defines : `STM32WL55xx` `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `CORE_CM4` `USE_HAL_DRIVER`
  - includes : `src/kernel/core/ucore/embOSCXM4_518/inc`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `src/kernel/core/ucore/cmsis/Device/st/stm32wlxx`, `src/kernel/dev/arch/cortexm/stm32wlxx/cubemx_hal_driver/inc`, `src/kernel/dev/arch/cortexm/stm32wlxx/dev_stm32wlxx`, `src/kernel/dev/arch/cortexm/stm32wlxx/radio_subghz_phy`, `src/kernel/dev/arch/cortexm/stm32wlxx/radio_subghz_phy/stm32_radio_driver`, `src/kernel/dev/arch/cortexm/stm32wlxx/radio_subghz_phy/stm32_radio_target`, `src/kernel/dev/arch/cortexm/stm32wlxx/radio_subghz_phy/stm32_radio_core_inc`, `src/kernel/dev/arch/cortexm/stm32wlxx/Utilities/misc`, `src/kernel/dev/arch/cortexm/stm32wlxx/Utilities/timer`, `src/kernel/dev/arch/cortexm/stm32wlxx/Utilities/trace/adv_trace`, `src/kernel/dev/arch/cortexm/stm32wlxx/Utilities/sequencer`
- **Release** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 9.50.2.71646 ; sources c/cpp/asm 63/0/0, exclus 0, absents 0 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`

### `sys/root/prj/iar/arch/arm/stm32f4-usb-core/Backup of stm32f4_usb_core.ewp`

fileVersion 2 ; carte : STM32F4 (USB device) ; workspaces : aucun ; fichiers déclarés : 24 ; absents : 1.

- **Debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 10/0/0, exclus 2, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\stm32f4_usb_core.a`
  - defines : `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `USE_STDPERIPH_DRIVER` `STM32F429xx`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc`, `src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/legacy`, `src/kernel/usb/stm32f4-usb-core/core`, `src/kernel/usb/stm32f4-usb-core/stm32_usb_device_library/Core/Inc`, `src/kernel/usb/stm32f4-usb-core/stm32_usb_device_library/Class/AUDIO/Inc`
- **Release** — ? (indéterminé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 12/0/0, exclus 0, absents 1 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`

### `sys/root/prj/iar/arch/arm/stm32f4-usb-core/stm32f4_usb_core.ewp`

fileVersion 2 ; carte : STM32F4 (USB device) ; workspaces : aucun ; fichiers déclarés : 38 ; absents : 1.

- **Debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.80.4.12487 ; sources c/cpp/asm 17/0/0, exclus 2, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\stm32f4_usb_core.a`
  - defines : `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `USE_STDPERIPH_DRIVER` `STM32F429xx`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc`, `src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/legacy`, `src/kernel/usb/stm32f4-usb-core/core`, `src/kernel/usb/stm32f4-usb-core/stm32_usb_device_library/Core/Inc`, `src/kernel/usb/stm32f4-usb-core/stm32_usb_device_library/Class/AUDIO/Inc`, `src/kernel/usb/stm32f4-usb-core/stm32_usb_device_library/Class/MSC/Inc`
- **Release** — ? (indéterminé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 19/0/0, exclus 0, absents 1 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`

### `sys/root/prj/iar/arch/arm/stm32f4-usb-core/stm32f4_usb_core_8.40.ewp`

fileVersion 3 ; carte : STM32F4 (USB device) ; workspaces : aucun ; fichiers déclarés : 38 ; absents : 1.

- **Debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 8.40.1.21529 ; sources c/cpp/asm 17/0/0, exclus 2, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\stm32f4_usb_core.a`
  - defines : `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `USE_STDPERIPH_DRIVER` `STM32F429xx`
  - includes : `src/kernel/core/ucore/embOSCXM4_518/inc`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc`, `src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/legacy`, `src/kernel/usb/stm32f4-usb-core/core`, `src/kernel/usb/stm32f4-usb-core/stm32_usb_device_library/Core/Inc`, `src/kernel/usb/stm32f4-usb-core/stm32_usb_device_library/Class/AUDIO/Inc`, `src/kernel/usb/stm32f4-usb-core/stm32_usb_device_library/Class/MSC/Inc`
- **Release** — ? (indéterminé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 19/0/0, exclus 0, absents 1 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`

### `sys/root/prj/iar/arch/arm/tauon/tauon.ewp`

fileVersion 1 ; carte : noyau Lepton (multi-cœur) ; workspaces : `tauon.eww`, `tauon_6.10.eww` ; fichiers déclarés : 515 ; absents : 254.

- **lib-debug** — ARM926EJ-S (puce), FPU None (FPU=0) ; puce `AT91SAM9261EK \| Atmel AT91SAM9261EK` ; EW 4.42A ; sources c/cpp/asm 283/0/1, exclus 117, absents 128 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\lib-debug\lib\tauon.r79`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP`
  - includes : `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`

### `sys/root/prj/iar/arch/arm/tauon/tauon_6.10.ewp`

fileVersion 2 ; carte : noyau Lepton (multi-cœur) ; workspaces : aucun ; fichiers déclarés : 678 ; absents : 256.

- **lib-debug** — Cortex-M3 (nom-config-corrélé), FPU None (FPU=0) ; puce `LM3Sx9xx \| TexasInstruments LM3Sx9xx` ; EW 6.10.3.52260 ; sources c/cpp/asm 172/0/0, exclus 336, absents 4 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\lib-debug\lib\tauon_6.10.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`

### `sys/root/prj/iar/arch/arm/tauon/tauon_6.21.ewp`

fileVersion 2 ; carte : noyau Lepton (multi-cœur) ; workspaces : `tauon_6.21.eww`, `tauon_at91sam9261.eww`, `tauon-basic_cmsis_6.21.eww`, `tauon-basic_lm3s_6.21.eww`, `tauon-basic_stm32f4_6.21.eww` ; fichiers déclarés : 739 ; absents : 259.

- Includes communs : `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/uip2.5`, `sys/root/src`
- **lib-debug** — Cortex-M3 (nom-config-corrélé), FPU None (FPU=0) ; puce `LM3SxDxx \| TexasInstruments LM3SxDxx` ; EW 6.21.1.52845 ; sources c/cpp/asm 210/0/2, exclus 382, absents 12 ; bibliothèque → `$PROJ_DIR$\lib-debug\lib\tauon.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`
- **tauon-kernel-cortex-m4-debug** — Cortex-M4 (nom-config), FPU None (FPU=0) ; puce `LM3SxDxx \| TexasInstruments LM3SxDxx` ; EW 6.21.1.52845 ; sources c/cpp/asm 184/0/0, exclus 428, absents 12 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m4-debug\Exe\tauon_6.21.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`
- **tauon-kernel-arm926ejs-debug** — ARM926EJ-S (nom-config), FPU None (FPU=0) ; puce `LM3SxDxx \| TexasInstruments LM3SxDxx` ; EW 6.21.1.52845 ; sources c/cpp/asm 187/0/0, exclus 423, absents 12 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-arm926ejs-debug\Exe\tauon_6.21.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `SDRAM_TARGET=1`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `src/kernel/core/ucore/embOSARM7-9_388/inc`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`
- **tauon-kernel-cortex-m3-debug** — Cortex-M3 (nom-config), FPU None (FPU=0) ; puce `LM3SxDxx \| TexasInstruments LM3SxDxx` ; EW 6.21.1.52845 ; sources c/cpp/asm 175/0/0, exclus 387, absents 3 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m3-debug\Exe\tauon_6.21.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`
- **tauon-kernel-cortex-m4f-freertos-debug** — Cortex-M4 (nom-config), FPU FPU=5 (code IAR brut) ; puce `LM3SxDxx \| TexasInstruments LM3SxDxx` ; EW 6.21.1.52845 ; sources c/cpp/asm 157/0/1, exclus 448, absents 3 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m4f-freertos-debug\Exe\tauon_6.21.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`
- **tauon-kernel-arm926ejs-freertos-debug** — ARM926EJ-S (nom-config), FPU None (FPU=0) ; puce `LM3SxDxx \| TexasInstruments LM3SxDxx` ; EW 6.21.1.52845 ; sources c/cpp/asm 195/0/1, exclus 399, absents 12 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-arm926ejs-freertos-debug\Exe\tauon_6.21.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `SDRAM_TARGET=1`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `src/kernel/core/ucore/embOSARM7-9_388/inc`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`

### `sys/root/prj/iar/arch/arm/tauon/tauon_7.20.ewp`

fileVersion 2 ; carte : noyau Lepton (multi-cœur) ; workspaces : `tauon_at91sam9261_7.20.eww`, `tauon-basic_at91samd20_7.20.eww` ; fichiers déclarés : 769 ; absents : 257.

- Includes communs : `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/uip2.5`, `sys/root/src`
- **lib-debug** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 222/0/3, exclus 382, absents 11 ; bibliothèque → `$PROJ_DIR$\lib-debug\lib\tauon.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`
- **tauon-kernel-cortex-m4-debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.80.4.12487 ; sources c/cpp/asm 195/0/0, exclus 431, absents 11 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\tauon_7.20.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`
- **tauon-kernel-arm926ejs-debug** — ARM926EJ-S (nom-config), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 198/0/0, exclus 426, absents 11 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-arm926ejs-debug\Exe\tauon_6.21.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `SDRAM_TARGET=1`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `src/kernel/core/ucore/embOSARM7-9_388/inc`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`
- **tauon-kernel-cortex-m3-debug** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 186/0/0, exclus 390, absents 2 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m3-debug\Exe\tauon_6.21.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`
- **tauon-kernel-cortex-m4f-freertos-debug** — Cortex-M4 (xcl-corrélé), FPU FPU2=4 (code IAR brut) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 203/0/1, exclus 408, absents 11 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m4f-freertos-debug\Exe\tauon_7.20.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`
- **tauon-kernel-arm926ejs-freertos-debug** — ARM926EJ-S (nom-config), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 206/0/1, exclus 402, absents 11 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-arm926ejs-freertos-debug\Exe\tauon_7.20.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `SDRAM_TARGET=1`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `src/kernel/core/ucore/embOSARM7-9_388/inc`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`
- **tauon-kernel-cortex-m0+-freertos-debug** — Cortex-M0+ (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 203/0/1, exclus 408, absents 11 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m0+-freertos-debug\Exe\tauon_7.20.a`
  - defines : `ARM_MATH_CM0=true` `NDEBUG`
  - defines asm : `ARM_MATH_CM0=true`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM0`

### `sys/root/prj/iar/arch/arm/tauon/tauon_7.80.ewp`

fileVersion 2 ; carte : noyau Lepton (multi-cœur) ; workspaces : `tauon-basic_at91samv71_7.80.eww`, `tauon-basic_stm32f4-olimex_p407_7.20.eww`, `tauon-basic_stm32f407_discovery_7.20.eww`, `tauon-basic_stm32f469i_eval_7.80.eww` ; fichiers déclarés : 716 ; absents : 244.

- Includes communs : `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `sys/root/src`
- **lib-debug** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.80.4.12487 ; sources c/cpp/asm 244/0/3, exclus 281, absents 0 ; bibliothèque → `$PROJ_DIR$\lib-debug\lib\tauon.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`, `src/kernel/net/uip2.5`
- **tauon-kernel-cortex-m4-debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.80.4.12487 ; sources c/cpp/asm 217/0/0, exclus 330, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\tauon_7.80.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_440/Inc`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`, `src/kernel/net/uip/core`
- **tauon-kernel-arm926ejs-debug** — ARM926EJ-S (nom-config), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 220/0/0, exclus 325, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-arm926ejs-debug\Exe\tauon_6.21.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `SDRAM_TARGET=1`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `src/kernel/core/ucore/embOSARM7-9_388/inc`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`
- **tauon-kernel-cortex-m3-debug** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 176/0/0, exclus 379, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m3-debug\Exe\tauon_6.21.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`, `src/kernel/net/uip2.5`
- **tauon-kernel-cortex-m4f-freertos-debug** — Cortex-M4 (xcl-corrélé), FPU FPU2=4 (code IAR brut) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 225/0/1, exclus 307, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m4f-freertos-debug\Exe\tauon_7.20.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`, `src/kernel/net/uip2.5`
- **tauon-kernel-arm926ejs-freertos-debug** — ARM926EJ-S (nom-config), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 228/0/1, exclus 301, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-arm926ejs-freertos-debug\Exe\tauon_7.20.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `SDRAM_TARGET=1`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `src/kernel/core/ucore/embOSARM7-9_388/inc`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`
- **tauon-kernel-cortex-m0+-freertos-debug** — Cortex-M0+ (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.80.4.12487 ; sources c/cpp/asm 225/0/1, exclus 307, absents 0 ; bibliothèque → `C:\lepton\rootstock-mainline\depots\lepton\root\master\kernel\scion\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m0+-freertos-debug\Exe\tauon_7.80.a`
  - defines : `ARM_MATH_CM0=true` `NDEBUG`
  - defines asm : `ARM_MATH_CM0=true`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM0`, `src/kernel/net/uip2.5`
- **tauon-kernel-cortex-m4m7-freertosv9-debug** — Cortex-M7 (xcl-corrélé), FPU VFPv5_SP (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.80.4.12487 ; sources c/cpp/asm 217/0/0, exclus 330, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\tauon_7.80.a`
  - defines : `__SAMV71Q21__` `sram` `TRACE_LEVEL=4` `NDEBUG`
  - defines asm : `__SAM4S16C__` `sram` `__ASSEMBLY__`
  - includes : `src/kernel/core/ucore/freeRTOS_9-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/portable/IAR/ARM_CM7/r0p1`, `src/kernel/net/uip2.5`
- **tauon-kernel-cortex-m7-debug** — Cortex-M7 (xcl-corrélé), FPU VFPv5_SP (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.80.4.12487 ; sources c/cpp/asm 217/0/0, exclus 330, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\tauon_7.80.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/embOSCXM7_430/Inc`, `src/kernel/net/uip/core`

### `sys/root/prj/iar/arch/arm/tauon/tauon_8.40.ewp`

fileVersion 3 ; carte : noyau Lepton (multi-cœur) ; workspaces : `tauon-basic_stm32wl55jci_nucleo_8.40.eww` ; fichiers déclarés : 717 ; absents : 244.

- Includes communs : `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `sys/root/src`
- **lib-debug** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 8.40.1.21529 ; sources c/cpp/asm 245/0/3, exclus 281, absents 0 ; bibliothèque → `$PROJ_DIR$\lib-debug\lib\tauon.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`, `src/kernel/net/uip2.5`
- **tauon-kernel-cortex-m4-debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 8.40.1.21529 ; sources c/cpp/asm 218/0/0, exclus 330, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\tauon_8.40.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_518/inc`, `src/kernel/net/uip/core`
- **tauon-kernel-arm926ejs-debug** — ARM926EJ-S (nom-config), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 221/0/0, exclus 325, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-arm926ejs-debug\Exe\tauon_6.21.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `SDRAM_TARGET=1`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `src/kernel/core/ucore/embOSARM7-9_388/inc`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`
- **tauon-kernel-cortex-m3-debug** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 177/0/0, exclus 379, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m3-debug\Exe\tauon_6.21.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`, `src/kernel/net/uip2.5`
- **tauon-kernel-cortex-m4f-freertos-debug** — Cortex-M4 (xcl-corrélé), FPU FPU2=4 (code IAR brut) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 226/0/1, exclus 307, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m4f-freertos-debug\Exe\tauon_7.20.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`, `src/kernel/net/uip2.5`
- **tauon-kernel-arm926ejs-freertos-debug** — ARM926EJ-S (nom-config), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.40.3.8937 ; sources c/cpp/asm 229/0/1, exclus 301, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-arm926ejs-freertos-debug\Exe\tauon_7.20.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `SDRAM_TARGET=1`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `src/kernel/core/ucore/embOSARM7-9_388/inc`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`
- **tauon-kernel-cortex-m0+-freertos-debug** — Cortex-M0+ (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.80.4.12487 ; sources c/cpp/asm 226/0/1, exclus 307, absents 0 ; bibliothèque → `C:\lepton\rootstock-mainline\depots\lepton\root\master\kernel\scion\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m0+-freertos-debug\Exe\tauon_7.80.a`
  - defines : `ARM_MATH_CM0=true` `NDEBUG`
  - defines asm : `ARM_MATH_CM0=true`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM0`, `src/kernel/net/uip2.5`
- **tauon-kernel-cortex-m4m7-freertosv9-debug** — Cortex-M7 (xcl-corrélé), FPU VFPv5_SP (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.80.4.12487 ; sources c/cpp/asm 218/0/0, exclus 330, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\tauon_7.80.a`
  - defines : `__SAMV71Q21__` `sram` `TRACE_LEVEL=4` `NDEBUG`
  - defines asm : `__SAM4S16C__` `sram` `__ASSEMBLY__`
  - includes : `src/kernel/core/ucore/freeRTOS_9-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/portable/IAR/ARM_CM7/r0p1`, `src/kernel/net/uip2.5`
- **tauon-kernel-cortex-m7-debug** — Cortex-M7 (xcl-corrélé), FPU VFPv5_SP (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 7.80.4.12487 ; sources c/cpp/asm 218/0/0, exclus 330, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\tauon_7.80.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/embOSCXM7_430/Inc`, `src/kernel/net/uip/core`

### `sys/root/prj/iar/arch/arm/tauon/tauon_9.50.ewp`

fileVersion 4 ; carte : noyau Lepton (multi-cœur) ; workspaces : `tauon-basic_stm32wl55jci_nucleo_9.50.eww` ; fichiers déclarés : 716 ; absents : 244.

- Includes communs : `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `sys/root/src`
- **lib-debug** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 9.50.2.71646 ; sources c/cpp/asm 244/0/3, exclus 281, absents 0 ; bibliothèque → `$PROJ_DIR$\lib-debug\lib\tauon.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`, `src/kernel/net/uip2.5`
- **tauon-kernel-cortex-m4-debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 9.50.2.71646 ; sources c/cpp/asm 217/0/0, exclus 330, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\tauon_8.40.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_518/inc`, `src/kernel/core/ucore/embOSCXM4_440/Inc`, `src/kernel/net/uip/core`
- **tauon-kernel-arm926ejs-debug** — ARM926EJ-S (nom-config), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 9.50.2.71646 ; sources c/cpp/asm 220/0/0, exclus 325, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-arm926ejs-debug\Exe\tauon_6.21.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `SDRAM_TARGET=1`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `src/kernel/core/ucore/embOSARM7-9_388/inc`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`
- **tauon-kernel-cortex-m3-debug** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 9.50.2.71646 ; sources c/cpp/asm 176/0/0, exclus 379, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m3-debug\Exe\tauon_6.21.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`, `src/kernel/net/uip2.5`
- **tauon-kernel-cortex-m4f-freertos-debug** — Cortex-M4 (xcl-corrélé), FPU FPU2=4 (code IAR brut) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 9.50.2.71646 ; sources c/cpp/asm 225/0/1, exclus 307, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m4f-freertos-debug\Exe\tauon_7.20.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/embOSARM7_360`, `src/kernel/core/ucore/embOSARM7_360/src/cpu (absent)`, `src/kernel/core/ucore/embOSARM7_360/src/generic (absent)`, `src/kernel/net/uip2.5`
- **tauon-kernel-arm926ejs-freertos-debug** — ARM926EJ-S (nom-config), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 9.50.2.71646 ; sources c/cpp/asm 228/0/1, exclus 301, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-arm926ejs-freertos-debug\Exe\tauon_7.20.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `SDRAM_TARGET=1`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `src/kernel/core/ucore/embOSARM7-9_388/inc`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`
- **tauon-kernel-cortex-m0+-freertos-debug** — Cortex-M0+ (xcl-corrélé), FPU None (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 9.50.2.71646 ; sources c/cpp/asm 225/0/1, exclus 307, absents 0 ; bibliothèque → `C:\lepton\rootstock-mainline\depots\lepton\root\master\kernel\scion\sys\root\prj\iar\arch\arm\tauon\tauon-kernel-cortex-m0+-freertos-debug\Exe\tauon_7.80.a`
  - defines : `ARM_MATH_CM0=true` `NDEBUG`
  - defines asm : `ARM_MATH_CM0=true`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM0`, `src/kernel/net/uip2.5`
- **tauon-kernel-cortex-m4m7-freertosv9-debug** — Cortex-M7 (xcl-corrélé), FPU VFPv5_SP (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 9.50.2.71646 ; sources c/cpp/asm 217/0/0, exclus 330, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\tauon_7.80.a`
  - defines : `__SAMV71Q21__` `sram` `TRACE_LEVEL=4` `NDEBUG`
  - defines asm : `__SAM4S16C__` `sram` `__ASSEMBLY__`
  - includes : `src/kernel/core/ucore/freeRTOS_9-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/portable/IAR/ARM_CM7/r0p1`, `src/kernel/net/uip2.5`
- **tauon-kernel-cortex-m7-debug** — Cortex-M7 (xcl-corrélé), FPU VFPv5_SP (xcl-corrélé) ; puce `LM3S9D96 \| TexasInstruments LM3S9D96` ; EW 9.50.2.71646 ; sources c/cpp/asm 217/0/0, exclus 330, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\tauon_7.80.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/embOSCXM7_430/Inc`, `src/kernel/net/uip/core`

### `sys/root/prj/iar/bsp/discovery_f4-baseboard-modem/bsp_discovery_f4-baseboard-modem_7.40.ewp`

fileVersion 2 ; carte : STM32F4 Discovery + modem ; workspaces : aucun ; fichiers déclarés : 18 ; absents : 0.

- **Debug** — Cortex-M4 (xcl), FPU None (xcl) ; puce `Default \| None` ; EW 7.80.4.12487 ; sources c/cpp/asm 14/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\bsp_discovery_f4-baseboard-modem_7.40.a`
  - defines : `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `USE_STDPERIPH_DRIVER`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc`, `src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/legacy`
- **Release** — ? (indéterminé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 14/0/0, exclus 0, absents 0 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`

### `sys/root/prj/iar/bsp/discovery_f4/bsp_discovery_f4_7.30.ewp`

fileVersion 2 ; carte : STM32F4-Discovery ; workspaces : `tauon-basic_stm32f407_discovery_7.20.eww`, `tauon-basic_stm32f469i_eval_7.80.eww` ; fichiers déclarés : 3 ; absents : 0.

- **Debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 2/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Exe\bsp_discovery_f4_7.30.a`
  - defines : `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `USE_STDPERIPH_DRIVER`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`
- **Release** — ? (indéterminé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 2/0/0, exclus 0, absents 0 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`

### `sys/root/prj/iar/bsp/olimex_p407/bsp_olimex_p407_7.30.ewp`

fileVersion 2 ; carte : Olimex STM32-P407 ; workspaces : `tauon-basic_stm32f4-olimex_p407_7.20.eww` ; fichiers déclarés : 4 ; absents : 0.

- **Debug** — Cortex-M4 (xcl), FPU None (xcl) ; puce `Default \| None` ; EW 7.80.4.12487 ; sources c/cpp/asm 3/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Exe\bsp_olimex_p407_7.30.a`
  - defines : `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `USE_STDPERIPH_DRIVER`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`
- **Release** — ? (indéterminé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.80.4.12487 ; sources c/cpp/asm 3/0/0, exclus 0, absents 0 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`

### `sys/root/prj/iar/bsp/samd20xplained_pro/bsp_samd20xplained_pro_7.30.ewp`

fileVersion 2 ; carte : SAMD20 Xplained Pro ; workspaces : `tauon-basic_at91samd20_7.20.eww` ; fichiers déclarés : 6 ; absents : 0.

- **Debug** — Cortex-M0+ (xcl), FPU None (xcl) ; puce `Default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 4/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Exe\bsp_samd20xplained_pro_7.30.a`
  - defines : `ARM_MATH_CM0=true` `_BOARD=SAMD20_XPLAINED_PRO` `EVENTS_INTERRUPT_HOOKS_MODE=true` `TC_ASYNC=true` `USART_CALLBACK_MODE=true` `__SAMD20J18__`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM0`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include`, `src/kernel/dev/arch/at91/asf/sam0/utils`, `src/kernel/dev/arch/at91/asf/sam0/utils/preprocessor`, `src/kernel/dev/arch/at91/asf/sam0/boards (absent)`, `src/kernel/dev/arch/at91/asf/sam0/boards/samd20_xplained_pro (absent)`, `src/kernel/dev/arch/at91/asf/sam0/drivers/port`, `src/kernel/dev/arch/at91/asf/sam0/drivers/sercom`, `src/kernel/dev/arch/at91/asf/sam0/drivers/tc`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/interrupt`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/interrupt/system_interrupt_samd20`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/clock`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/clock/clock_samd20`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/pinmux`, `src/kernel/dev/arch/at91/asf/sam0/drivers/events`, `src/kernel/dev/arch/at91/asf/common/utils`, `src/kernel/dev/arch/at91/asf/common/boards`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/source`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include/component`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include/pio`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include/instance`, `src/kernel/dev/arch/at91/asf/sam0/utils/header_files`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/clock/clock_samd20/module_config`
- **Release** — ? (indéterminé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 4/0/0, exclus 0, absents 0 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`

### `sys/root/prj/iar/bsp/same70xplained/bsp_same70xplained_7.80.ewp`

fileVersion 2 ; carte : SAME70 Xplained ; workspaces : aucun ; fichiers déclarés : 5 ; absents : 0.

- **Debug** — Cortex-M3 (xcl), FPU None (xcl) ; puce `Default \| None` ; EW 7.80.4.12487 ; sources c/cpp/asm 4/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\bsp\same70xplained\Debug\Exe\bsp_same70xplained_7.80.a`
- **Release** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `-` ; EW 7.80.4.12487 ; sources c/cpp/asm 4/0/0, exclus 0, absents 0 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`
- **freertos-debug** — Cortex-M7 (xcl), FPU VFPv5_SP (xcl) ; puce `ATSAMV71Q21 \| Atmel ATSAMV71Q21` ; EW 7.80.4.12487 ; sources c/cpp/asm 2/0/0, exclus 2, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\bsp_same70xplained_7.80.a`
  - defines : `__SAMV71Q21__` `__LEPTON_SAME70_REVB__` `BOARD_SAMV71_XULT` `MPU_HAS_NOCACHE_REGION` `sram` `TRACE_LEVEL=0` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/embOSCXM7_430/Inc`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/portable/IAR/ARM_CM7/r0p1`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip/include`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip/include/samv71`, `src/kernel/dev/arch/at91/softpack-lib/samv71/samv71-xplained-ultra/libboard`, `src/kernel/dev/arch/at91/softpack-lib/samv71/samv71-xplained-ultra/libboard/include`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia/include`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia/include/nandflash`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia/include/sdmmc`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia/include/spiflash`

### `sys/root/prj/iar/bsp/samv71xplained_ultra/bsp_samv71xplained_ultra_7.80.ewp`

fileVersion 2 ; carte : SAMV71 Xplained Ultra ; workspaces : `tauon-basic_at91samv71_7.80.eww` ; fichiers déclarés : 7 ; absents : 0.

- **Debug** — Cortex-M3 (xcl), FPU None (xcl) ; puce `Default \| None` ; EW 7.80.4.12487 ; sources c/cpp/asm 6/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\sys\root\prj\iar\bsp\samv71xplained_ultra\Debug\Exe\bsp_samv71xplained_ultra_7.80.a`
- **Release** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `-` ; EW 7.80.4.12487 ; sources c/cpp/asm 6/0/0, exclus 0, absents 0 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`
- **freertos-debug** — Cortex-M7 (xcl), FPU VFPv5_SP (xcl) ; puce `ATSAMV71Q21 \| Atmel ATSAMV71Q21` ; EW 7.80.4.12487 ; sources c/cpp/asm 6/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\bsp_samv71xplained_ultra_7.80.a`
  - defines : `__SAMV71Q21__` `__LEPTON_SAME70_REVB__` `BOARD_SAMV71_XULT` `MPU_HAS_NOCACHE_REGION` `sram` `TRACE_LEVEL=0` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/embOSCXM7_430/Inc`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/portable/IAR/ARM_CM7/r0p1`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip/include`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip/include/samv71`, `src/kernel/dev/arch/at91/softpack-lib/samv71/samv71-xplained-ultra/libboard`, `src/kernel/dev/arch/at91/softpack-lib/samv71/samv71-xplained-ultra/libboard/include`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia/include`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia/include/nandflash`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia/include/sdmmc`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libstoragemedia/include/spiflash`

### `sys/root/prj/iar/bsp/stm32f469i-eval/bsp-stm32f469i-eval.ewp`

fileVersion 2 ; carte : STM32F469I-EVAL ; workspaces : `tauon-basic_stm32f469i_eval_7.80.eww` ; fichiers déclarés : 6 ; absents : 0.

- **Debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.80.4.12487 ; sources c/cpp/asm 2/0/0, exclus 3, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Exe\bsp-stm32f469i-eval.a`
  - defines : `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `USE_STDPERIPH_DRIVER`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`
- **Release** — ? (indéterminé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 5/0/0, exclus 0, absents 0 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`

### `sys/root/prj/iar/bsp/stm32wl55jci_nucleo/bsp_stm32wl55jci_nucleo_8.40.ewp`

fileVersion 3 ; carte : STM32WL55 Nucleo ; workspaces : `tauon-basic_stm32wl55jci_nucleo_8.40.eww` ; fichiers déclarés : 8 ; absents : 0.

- **Debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 8.40.1.21529 ; sources c/cpp/asm 4/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\bsp_stm32wl55jci_nucleo_8.40.a`
  - defines : `STM32WL55xx` `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `CORE_CM4` `USE_HAL_DRIVER`
  - includes : `src/kernel/core/ucore/embOSCXM4_518/inc`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `src/kernel/core/ucore/cmsis/Device/st/stm32wlxx`, `src/kernel/dev/arch/cortexm/stm32wlxx/cubemx_hal_driver/inc`, `src/kernel/dev/arch/cortexm/stm32wlxx/dev_stm32wlxx`
- **Release** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `-` ; EW 8.40.1.21529 ; sources c/cpp/asm 4/0/0, exclus 0, absents 0 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`

### `sys/root/prj/iar/bsp/stm32wl55jci_nucleo/bsp_stm32wl55jci_nucleo_9.50.ewp`

fileVersion 4 ; carte : STM32WL55 Nucleo ; workspaces : `tauon-basic_stm32wl55jci_nucleo_9.50.eww` ; fichiers déclarés : 8 ; absents : 0.

- **Debug** — Cortex-M4 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `Default \| None` ; EW 9.50.2.71646 ; sources c/cpp/asm 4/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Exe\bsp_stm32wl55jci_nucleo_9.50.a`
  - defines : `STM32WL55xx` `ewarm` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG` `CORE_CM4` `USE_HAL_DRIVER`
  - includes : `src/kernel/core/ucore/embOSCXM4_518/inc`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `src/kernel/core/ucore/cmsis/Device/st/stm32wlxx`, `src/kernel/dev/arch/cortexm/stm32wlxx/cubemx_hal_driver/inc`, `src/kernel/dev/arch/cortexm/stm32wlxx/dev_stm32wlxx`
- **Release** — Cortex-M3 (xcl-corrélé), FPU None (xcl-corrélé) ; puce `-` ; EW 9.50.2.71646 ; sources c/cpp/asm 4/0/0, exclus 0, absents 0 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `NDEBUG`

### `sys/root/prj/iar/lib/lib-nxpnfc/lib-nxpnfc.ewp`

fileVersion 2 ; carte : NXP PN7150 (NFC) ; workspaces : aucun ; fichiers déclarés : 24 ; absents : 0.

- Includes communs : `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/lib/lib-nxpnfc`, `src/lib/lib-nxpnfc/NfcLibrary/NdefLibrary/inc`, `src/lib/lib-nxpnfc/NfcLibrary/NxpNci/inc`, `src/lib/lib-nxpnfc/NfcLibrary/inc`, `src/lib/lib-nxpnfc/tml`, `src/lib/lib-nxpnfc/tool`, `sys/root/src`
- **cortex-m4-debug** — Cortex-M4 (xcl), FPU None (xcl) ; puce `Default \| None` ; EW 7.80.4.12487 ; sources c/cpp/asm 11/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\lib-nxpnfc.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `RW_SUPPORT`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `src/kernel/core/ucore/cmsis/CMSIS/Include`
- **cortex-m7-debug** — Cortex-M7 (xcl), FPU VFPv5_SP (xcl) ; puce `Default \| None` ; EW 7.80.4.12487 ; sources c/cpp/asm 11/0/0, exclus 0, absents 0 ; bibliothèque → `C:\tauon\building\output\$PROJ_FNAME$\$CONFIG_NAME$\Lib\lib-nxpnfc.a`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `RW_SUPPORT`
  - includes : `src/kernel/core/ucore/embOSCXM7_430/Inc`

### `sys/root/src/kernel/dev/arch/cortexm/stellaris/driverlib/driverlib.ewp`

fileVersion 2 ; carte : Stellaris LM3S ; workspaces : aucun ; fichiers déclarés : 23 ; absents : 0.

- **Debug** — Cortex-M3 (puce), FPU None (FPU=0) ; puce `LM3SxDxx \| TexasInstruments LM3SxDxx` ; EW 6.21.1.52845 ; sources c/cpp/asm 23/0/0, exclus 0, absents 0 ; bibliothèque → `C:\StellarisWare\driverlib\ewarm\Exe\driverlib.a`
  - defines : `ewarm`
  - defines asm : `ewarm`
  - includes : `src/kernel/dev/arch/cortexm/stellaris`

### `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic-stm32wl55jci-nucleo/tauon-basic_stm32wl55jci_nucleo_8.40.ewp`

fileVersion 3 ; carte : STM32WL55 Nucleo ; workspaces : `tauon-basic_stm32wl55jci_nucleo_8.40.eww` ; fichiers déclarés : 70 ; absents : 10.

- **Debug** — Cortex-M4 (puce), FPU None (xcl-corrélé) ; puce `STM32WL55JC_M4 \| ST STM32WL55JC_M4` ; EW 8.40.1.21529 ; sources c/cpp/asm 13/0/2, exclus 38, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic-stm32wl55jci-nucleo/stm32wl55xC_m4-lepton.icf`
  - defines : `STM32WL55xx` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG=1` `ewarm`
  - includes : `src/kernel/core/ucore/embOSCXM4_518/inc`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`
  - bibliothèques liées : `building/output/bsp_stm32wl55jci_nucleo_8.40/Debug/Lib/bsp_stm32wl55jci_nucleo_8.40.a`, `building/output/dev_stm32wlxx_8.40/Debug/Lib/dev_stm32wlxx_8.40.a`, `building/output/tauon_8.40/tauon-kernel-cortex-m4-debug/Lib/tauon_8.40.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_518/lib/os7m_tl__sp.a`
- **Release** — Cortex-M3 (puce), FPU None (xcl-corrélé) ; puce `LM3S9B96 \| TexasInstruments LM3S9B96` ; EW 7.80.4.12487 ; sources c/cpp/asm 39/0/4, exclus 0, absents 6 ; ICF par défaut EWARM (`$TOOLKIT_DIR$\CONFIG\generic_cortex.icf`)
  - includes : `sys/user/tauon-basic/prj/iar/Inc (absent)`
  - bibliothèques liées : `building/output/bsp_stm32wl55jci_nucleo_8.40/Debug/Lib/bsp_stm32wl55jci_nucleo_8.40.a`, `building/output/dev_stm32wlxx_8.40/Debug/Lib/dev_stm32wlxx_8.40.a`, `building/output/tauon_8.40/tauon-kernel-cortex-m4-debug/Lib/tauon_8.40.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_518/lib/os7m_tl__sp.a`
- **freertos-debug** — Cortex-M3 (puce), FPU None (xcl-corrélé) ; puce `STM32F207ZG \| ST STM32F207ZG` ; EW 8.40.1.21529 ; sources c/cpp/asm 26/0/3, exclus 20, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic-stm32wl55jci-nucleo/FLASH.icf` (absent)
  - defines : `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG=1` `ewarm` `PART_LM3S9B96`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM4_386/arch/cmsis/DeviceSupport`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/Device/st/stm32f4xx`
  - bibliothèques liées : `building/output/bsp_stm32wl55jci_nucleo_8.40/Debug/Lib/bsp_stm32wl55jci_nucleo_8.40.a`, `building/output/dev_stm32wlxx_8.40/Debug/Lib/dev_stm32wlxx_8.40.a`, `building/output/tauon_8.40/tauon-kernel-cortex-m4-debug/Lib/tauon_8.40.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_518/lib/os7m_tl__sp.a`

### `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic-stm32wl55jci-nucleo/tauon-basic_stm32wl55jci_nucleo_9.50.ewp`

fileVersion 4 ; carte : STM32WL55 Nucleo ; workspaces : `tauon-basic_stm32wl55jci_nucleo_9.50.eww` ; fichiers déclarés : 71 ; absents : 10.

- **Debug** — Cortex-M4 (puce), FPU None (xcl-corrélé) ; puce `STM32WL55JC_M4 \| ST STM32WL55JC_M4` ; EW 9.50.2.71646 ; sources c/cpp/asm 13/0/2, exclus 39, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic-stm32wl55jci-nucleo/stm32wl55xC_m4-lepton.icf`
  - defines : `STM32WL55xx` `_OS_LIBMODE_DP` `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG=1` `ewarm` `CORE_CM4` `USE_HAL_DRIVER`
  - includes : `src/kernel/core/ucore/embOSCXM4_518/inc`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/Device/st/stm32wlxx`, `src/kernel/dev/arch/cortexm/stm32wlxx/dev_stm32wlxx`, `src/kernel/dev/arch/cortexm/stm32wlxx/cubemx_hal_driver/inc`
  - bibliothèques liées : `building/output/bsp_stm32wl55jci_nucleo_9.50/Debug/Lib/bsp_stm32wl55jci_nucleo_9.50.a`, `building/output/dev_stm32wlxx_9.50/Debug/Lib/dev_stm32wlxx_9.50.a`, `building/output/tauon_9.50/tauon-kernel-cortex-m4-debug/Lib/tauon_9.50.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_518/lib/os7m_tl__sp.a`
- **Release** — Cortex-M3 (puce), FPU None (xcl-corrélé) ; puce `LM3S9B96 \| TexasInstruments LM3S9B96` ; EW 9.50.2.71646 ; sources c/cpp/asm 39/0/4, exclus 0, absents 6 ; ICF par défaut EWARM (`$TOOLKIT_DIR$\CONFIG\generic_cortex.icf`)
  - includes : `sys/user/tauon-basic/prj/iar/Inc (absent)`
  - bibliothèques liées : `building/output/bsp_stm32wl55jci_nucleo_9.50/Debug/Lib/bsp_stm32wl55jci_nucleo_9.50.a`, `building/output/dev_stm32wlxx_9.50/Debug/Lib/dev_stm32wlxx_9.50.a`, `building/output/tauon_9.50/tauon-kernel-cortex-m4-debug/Lib/tauon_9.50.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_518/lib/os7m_tl__dp.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_518/lib/os7m_tl__sp.a`
- **freertos-debug** — Cortex-M3 (puce), FPU None (xcl-corrélé) ; puce `STM32F207ZG \| ST STM32F207ZG` ; EW 9.50.2.71646 ; sources c/cpp/asm 26/0/3, exclus 20, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic-stm32wl55jci-nucleo/FLASH.icf` (absent)
  - defines : `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG=1` `ewarm` `PART_LM3S9B96`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM4_386/arch/cmsis/DeviceSupport`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/Device/st/stm32f4xx`
  - bibliothèques liées : `building/output/bsp_stm32wl55jci_nucleo_9.50/Debug/Lib/bsp_stm32wl55jci_nucleo_9.50.a`, `building/output/dev_stm32wlxx_9.50/Debug/Lib/dev_stm32wlxx_9.50.a`, `building/output/tauon_9.50/tauon-kernel-cortex-m4-debug/Lib/tauon_9.50.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_518/lib/os7m_tl__dp.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_518/lib/os7m_tl__sp.a`

### `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91sam9261/tauon_at91sam9261.ewp`

fileVersion 2 ; carte : AT91SAM9261 (ARM9) ; workspaces : `tauon_at91sam9261.eww` ; fichiers déclarés : 70 ; absents : 22.

- **firmware** — ARM7TDMI (puce), FPU None (FPU=0) ; puce `AT91M55800 \| Atmel AT91M55800` ; EW 6.21.1.52845 ; sources c/cpp/asm 26/0/2, exclus 24, absents 4 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP`
  - defines asm : `TARGET_RAM`
  - includes : `src/kernel/core/ucore/embOSARM7-9_388/inc`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`
  - bibliothèques liées : `sys/root/prj/iar/arch/arm/dev/at91sam9261/lib-debug/lib/dev_at91sam9261.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-debug/Exe/tauon_6.21.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-freertos-debug/Exe/tauon_6.21.a`, `sys/root/src/kernel/core/ucore/embOSARM7-9_388/lib/os5t_al_isp.a`
- **firmware_injector** — ARM926EJ-S (puce), FPU None (FPU=0) ; puce `AT91SAM9261 \| Atmel AT91SAM9261` ; EW 6.21.1.52845 ; sources c/cpp/asm 19/0/1, exclus 37, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91sam9261/xcl_mac_at91sam9261/AT91SAM9261_SDRAM.icf`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `SDRAM_TARGET=1`
  - defines asm : `TARGET_JTAG`
  - includes : `src/kernel/core/ucore/embOSARM7-9_388/inc`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`
  - bibliothèques liées : `sys/root/prj/iar/arch/arm/dev/at91sam9261/lib-debug/lib/dev_at91sam9261.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-debug/Exe/tauon_6.21.a`, `sys/root/src/kernel/core/ucore/embOSARM7-9_388/lib/os5t_al_isp.a`
- **firmware_flash** — ARM7TDMI (puce), FPU None (FPU=0) ; puce `AT91M55800 \| Atmel AT91M55800` ; EW 6.21.1.52845 ; sources c/cpp/asm 33/0/4, exclus 0, absents 10 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `USE_XGUI` `DFLT_BRDBEEP_LVL=BRDBEEP_LVL_HIG`
  - defines asm : `TARGET_ROM`
  - includes : `X:\sources\embOSARM7_332 [externe x:]`, `X:\sources\embOSARM7_332\segger src\cpu [externe x:]`, `X:\sources\embOSARM7_332\segger src\generic [externe x:]`, `X:\sources [externe x:]`, `X:\sources\libc [externe x:]`, `X:\sources\libca [externe x:]`, `X:\sources\libmsr [externe x:]`, `X:\sources\lwip [externe x:]`, `X:\sources\lwip\include [externe x:]`, `X:\sources\lwip\ports\arm7 [externe x:]`, `X:\sources\lwip\ports\arm7\include [externe x:]`, `X:\sources\lwip\include\ipv4 [externe x:]`
  - bibliothèques liées : `sys/root/prj/iar/arch/arm/dev/at91sam9261/lib-debug/lib/dev_at91sam9261.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-debug/Exe/tauon_6.21.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-freertos-debug/Exe/tauon_6.21.a`, `sys/root/src/kernel/core/ucore/embOSARM7-9_388/lib/os5t_al_isp.a`
- **freertos-firmware_injector** — ARM926EJ-S (puce), FPU None (FPU=0) ; puce `AT91SAM9261 \| Atmel AT91SAM9261` ; EW 6.21.1.52845 ; sources c/cpp/asm 16/0/1, exclus 36, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91sam9261/xcl_mac_at91sam9261/AT91SAM9261_SDRAM_FREERTOS.icf` (absent)
  - defines : `NDEBUG` `SDRAM_TARGET=1` `at91sam9261` `flash_disable` `sdram` `SAM9XE_IAR`
  - defines asm : `at91sam9261` `TARGET_JTAG` `at91sam9xe512_disable` `sdram`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`
  - bibliothèques liées : `sys/root/prj/iar/arch/arm/dev/at91sam9261/lib-debug/lib/dev_at91sam9261.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-debug/Exe/tauon_6.21.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-freertos-debug/Exe/tauon_6.21.a`

### `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91sam9261/tauon_at91sam9261_7.20.ewp`

fileVersion 2 ; carte : AT91SAM9261 (ARM9) ; workspaces : `tauon_at91sam9261_7.20.eww` ; fichiers déclarés : 73 ; absents : 22.

- **firmware** — ARM7TDMI (puce), FPU None (FPU=0) ; puce `AT91M55800 \| Atmel AT91M55800` ; EW 6.21.1.52845 ; sources c/cpp/asm 26/0/2, exclus 24, absents 4 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP`
  - defines asm : `TARGET_RAM`
  - includes : `src/kernel/core/ucore/embOSARM7-9_388/inc`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`
  - bibliothèques liées : `sys/root/prj/iar/arch/arm/dev/at91sam9261/lib-debug/lib/dev_at91sam9261.a`, `sys/root/prj/iar/arch/arm/dev/at91sam9261/lib-debug/lib/dev_at91sam9261_7.20.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-debug/Exe/tauon_6.21.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-debug/Exe/tauon_7.20.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-freertos-debug/Exe/tauon_6.21.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-freertos-debug/Exe/tauon_7.20.a`, `sys/root/src/kernel/core/ucore/embOSARM7-9_388/lib/os5t_al_isp.a`
- **firmware_injector** — ARM926EJ-S (puce), FPU None (FPU=0) ; puce `AT91SAM9261 \| Atmel AT91SAM9261` ; EW 7.20.1.7306 ; sources c/cpp/asm 19/0/1, exclus 39, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91sam9261/xcl_mac_at91sam9261/AT91SAM9261_SDRAM.icf`
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG` `SDRAM_TARGET=1`
  - defines asm : `TARGET_JTAG`
  - includes : `src/kernel/core/ucore/embOSARM7-9_388/inc`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`
  - bibliothèques liées : `sys/root/prj/iar/arch/arm/dev/at91sam9261/lib-debug/lib/dev_at91sam9261_7.20.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-debug/Exe/tauon_7.20.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-freertos-debug/Exe/tauon_7.20.a`, `sys/root/src/kernel/core/ucore/embOSARM7-9_388/lib/os5t_al_isp.a`
- **firmware_flash** — ARM7TDMI (puce), FPU None (FPU=0) ; puce `AT91M55800 \| Atmel AT91M55800` ; EW 6.21.1.52845 ; sources c/cpp/asm 33/0/4, exclus 0, absents 10 ; ICF par défaut EWARM (`lnk0t.icf`)
  - defines : `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `USE_XGUI` `DFLT_BRDBEEP_LVL=BRDBEEP_LVL_HIG`
  - defines asm : `TARGET_ROM`
  - includes : `X:\sources\embOSARM7_332 [externe x:]`, `X:\sources\embOSARM7_332\segger src\cpu [externe x:]`, `X:\sources\embOSARM7_332\segger src\generic [externe x:]`, `X:\sources [externe x:]`, `X:\sources\libc [externe x:]`, `X:\sources\libca [externe x:]`, `X:\sources\libmsr [externe x:]`, `X:\sources\lwip [externe x:]`, `X:\sources\lwip\include [externe x:]`, `X:\sources\lwip\ports\arm7 [externe x:]`, `X:\sources\lwip\ports\arm7\include [externe x:]`, `X:\sources\lwip\include\ipv4 [externe x:]`
  - bibliothèques liées : `sys/root/prj/iar/arch/arm/dev/at91sam9261/lib-debug/lib/dev_at91sam9261.a`, `sys/root/prj/iar/arch/arm/dev/at91sam9261/lib-debug/lib/dev_at91sam9261_7.20.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-debug/Exe/tauon_6.21.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-debug/Exe/tauon_7.20.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-freertos-debug/Exe/tauon_6.21.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-freertos-debug/Exe/tauon_7.20.a`, `sys/root/src/kernel/core/ucore/embOSARM7-9_388/lib/os5t_al_isp.a`
- **freertos-firmware_injector** — ARM926EJ-S (puce), FPU None (FPU=0) ; puce `AT91SAM9261 \| Atmel AT91SAM9261` ; EW 7.20.1.7306 ; sources c/cpp/asm 16/0/1, exclus 40, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91sam9261/xcl_mac_at91sam9261/AT91SAM9261_SDRAM_FREERTOS.icf` (absent)
  - defines : `NDEBUG` `SDRAM_TARGET=1` `at91sam9261` `flash_disable` `sdram` `SAM9XE_IAR`
  - defines asm : `at91sam9261` `TARGET_JTAG` `at91sam9xe512_disable` `sdram`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/fs/yaffs/core`, `src/kernel/fs/yaffs/core/direct`, `src/kernel/net/uip2.5`, `src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek`, `src/kernel/dev/arch/at91/at91lib/peripherals`, `src/kernel/dev/arch/at91/at91lib/utility`
  - bibliothèques liées : `sys/root/prj/iar/arch/arm/dev/at91sam9261/lib-debug/lib/dev_at91sam9261_7.20.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-arm926ejs-freertos-debug/Exe/tauon_7.20.a`

### `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91samd20/tauon-basic_at91samd20_7.20.ewp`

fileVersion 2 ; carte : SAMD20 Xplained Pro ; workspaces : `tauon-basic_at91samd20_7.20.eww` ; fichiers déclarés : 21 ; absents : 5.

- **Debug** — Cortex-M0+ (puce), FPU None (xcl-corrélé) ; puce `ATSAMD20J18 \| Atmel ATSAMD20J18` ; EW 7.40.3.8937 ; sources c/cpp/asm 7/0/0, exclus 0, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91samd20/linker_scripts/samd20/iar/samd20j18_flash.icf`
  - defines : `ARM_MATH_CM0=true` `BOARD=SAMD20_XPLAINED_PRO` `EVENTS_INTERRUPT_HOOKS_MODE=true` `TC_ASYNC=true` `DAC_CALLBACK_MODE=true` `__SAMD20J18__`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/arch/cortex-m0+/at91samd20`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM0`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include`, `src/kernel/dev/arch/at91/asf/sam0/utils`, `src/kernel/dev/arch/at91/asf/sam0/utils/preprocessor`, `src/kernel/dev/arch/at91/asf/sam0/boards (absent)`, `src/kernel/dev/arch/at91/asf/sam0/boards/samd20_xplained_pro (absent)`, `src/kernel/dev/arch/at91/asf/sam0/drivers/port`, `src/kernel/dev/arch/at91/asf/sam0/drivers/dac (absent)`, `src/kernel/dev/arch/at91/asf/sam0/drivers/tc`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/interrupt`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/interrupt/system_interrupt_samd20`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/clock`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/clock/clock_samd20`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/pinmux`, `src/kernel/dev/arch/at91/asf/sam0/drivers/events`, `src/kernel/dev/arch/at91/asf/common/utils`, `src/kernel/dev/arch/at91/asf/common/boards`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/source`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include/component`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include/pio`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include/instance`, `src/kernel/dev/arch/at91/asf/sam0/utils/header_files`, `sys/user/tauon-basic/src/dev/board_atmel_at91samd20-xplained-pro/asf/sam0/boards (absent)`, `sys/user/tauon-basic/src/dev/board_atmel_at91samd20-xplained-pro/asf/sam0/boards/samd20_xplained_pro/board_config (absent)`
  - bibliothèques liées : `building/output/bsp_samd20xplained_pro_7.30/Debug/Exe/bsp_samd20xplained_pro_7.30.a`, `sys/root/prj/iar/arch/arm/dev/at91samd20/Debug/Exe/dev_at91samd20_7.20.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-cortex-m0+-freertos-debug/Exe/tauon_7.20.a`
- **Release** — ? (indéterminé), FPU None (xcl-corrélé) ; puce `default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 7/0/0, exclus 0, absents 3 ; ICF par défaut EWARM (`$TOOLKIT_DIR$\CONFIG\generic.icf`)
  - defines : `NDEBUG`
  - bibliothèques liées : `building/output/bsp_samd20xplained_pro_7.30/Debug/Exe/bsp_samd20xplained_pro_7.30.a`, `sys/root/prj/iar/arch/arm/dev/at91samd20/Debug/Exe/dev_at91samd20_7.20.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-cortex-m0+-freertos-debug/Exe/tauon_7.20.a`
- **freertos-debug** — Cortex-M0+ (puce), FPU None (xcl-corrélé) ; puce `ATSAMD20J18 \| Atmel ATSAMD20J18` ; EW 7.40.3.8937 ; sources c/cpp/asm 7/0/0, exclus 0, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91samd20/linker_scripts/samd20/iar/samd20j18_flash.icf`
  - defines : `ARM_MATH_CM0=true` `BOARD=SAMD20_XPLAINED_PRO` `EVENTS_INTERRUPT_HOOKS_MODE=true` `TC_ASYNC=true` `DAC_CALLBACK_MODE=true` `__SAMD20J18__`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/arch/cortex-m0+/at91samd20`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM0`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include`, `src/kernel/dev/arch/at91/asf/sam0/utils`, `src/kernel/dev/arch/at91/asf/sam0/utils/preprocessor`, `src/kernel/dev/arch/at91/asf/sam0/boards (absent)`, `src/kernel/dev/arch/at91/asf/sam0/boards/samd20_xplained_pro (absent)`, `src/kernel/dev/arch/at91/asf/sam0/drivers/port`, `src/kernel/dev/arch/at91/asf/sam0/drivers/dac (absent)`, `src/kernel/dev/arch/at91/asf/sam0/drivers/tc`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/interrupt`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/interrupt/system_interrupt_samd20`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/clock`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/clock/clock_samd20`, `src/kernel/dev/arch/at91/asf/sam0/drivers/system/pinmux`, `src/kernel/dev/arch/at91/asf/sam0/drivers/events`, `src/kernel/dev/arch/at91/asf/common/utils`, `src/kernel/dev/arch/at91/asf/common/boards`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/source`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include/component`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include/pio`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include`, `src/kernel/core/ucore/cmsis/Device/atmel/samd20/include/instance`, `src/kernel/dev/arch/at91/asf/sam0/utils/header_files`, `sys/user/tauon-basic/src/dev/board_atmel_at91samd20-xplained-pro/asf/sam0/boards (absent)`, `sys/user/tauon-basic/src/dev/board_atmel_at91samd20-xplained-pro/asf/sam0/boards/samd20_xplained_pro/board_config (absent)`
  - bibliothèques liées : `building/output/bsp_samd20xplained_pro_7.30/Debug/Exe/bsp_samd20xplained_pro_7.30.a`, `sys/root/prj/iar/arch/arm/dev/at91samd20/Debug/Exe/dev_at91samd20_7.20.a`, `sys/root/prj/iar/arch/arm/tauon/tauon-kernel-cortex-m0+-freertos-debug/Exe/tauon_7.20.a`

### `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91samv71/tauon-basic_at91samv71_7.80.ewp`

fileVersion 2 ; carte : SAMV71 Xplained Ultra ; workspaces : `tauon-basic_at91samv71_7.80.eww` ; fichiers déclarés : 28 ; absents : 6.

- **Debug** — Cortex-M7 (puce), FPU FPU2=7 (code IAR brut) ; puce `ATSAMV71Q21 \| Atmel ATSAMV71Q21` ; EW 7.80.4.12487 ; sources c/cpp/asm 19/0/1, exclus 2, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91samv71/linker_scripts/samv71/iar/samv71q21_flash.icf`
  - defines : `__SAMV71Q21__` `BOARD_SAMV71_XULT` `MPU_HAS_NOCACHE_REGION` `sram` `TRACE_LEVEL=0` `OS_SUPPORT_CLEANUP_ON_TERMINATE` `OS_LIBMODE_SP` `NDEBUG`
  - includes : `src/kernel/core/ucore/embOSCXM7_430/Inc`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip/include`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip/include/samv71`, `src/kernel/dev/arch/at91/softpack-lib/samv71/samv71-xplained-ultra/libboard`, `src/kernel/dev/arch/at91/softpack-lib/samv71/samv71-xplained-ultra/libboard/include`
  - bibliothèques liées : `building/output/bsp_samv71xplained_ultra_7.80/freertos-debug/Lib/bsp_samv71xplained_ultra_7.80.a`, `building/output/dev_at91samv7x_7.80/samv71-freertos-debug/Lib/dev_at91samv7x_7.80.a`, `building/output/tauon_7.80/tauon-kernel-cortex-m7-debug/Lib/tauon_7.80.a`, `sys/root/src/kernel/core/ucore/embOSCXM7_430/Lib/os7m_tlv_sp.a`
- **Release** — ? (indéterminé), FPU None (xcl-corrélé) ; puce `default \| None` ; EW 7.40.3.8937 ; sources c/cpp/asm 20/0/1, exclus 0, absents 4 ; ICF par défaut EWARM (`$TOOLKIT_DIR$\CONFIG\generic.icf`)
  - defines : `NDEBUG`
  - bibliothèques liées : `$PROJ_DIR$\..\..\..\..\..\..\..\..\..\lepton\rootstock-mainline\depots\lepton\root\master\sched\embos\scion\sys\root\src\kernel\core\ucore\embOSCXM7_430\Lib\os7m_tl__sp.a`, `building/output/bsp_samv71xplained_ultra_7.80/freertos-debug/Lib/bsp_samv71xplained_ultra_7.80.a`, `building/output/dev_at91samv7x_7.80/samv71-freertos-debug/Lib/dev_at91samv7x_7.80.a`, `building/output/tauon_7.80/tauon-kernel-cortex-m7-debug/Lib/tauon_7.80.a`, `sys/root/src/kernel/core/ucore/embOSCXM7_430/Lib/os7m_tlv_sp.a`
- **freertos-debug** — Cortex-M7 (puce), FPU VFPv5_SP (xcl-corrélé) ; puce `ATSAMV71Q21 \| Atmel ATSAMV71Q21` ; EW 7.80.4.12487 ; sources c/cpp/asm 20/0/1, exclus 0, absents 4 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_at91samv71/linker_scripts/samv71/iar/samv71q21_flash.icf`
  - defines : `__SAMV71Q21__` `BOARD_SAMV71_XULT` `sram` `TRACE_LEVEL=4`
  - includes : `src/kernel/core/ucore/freeRTOS_9-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_9-0-0/source/portable/IAR/ARM_CM7/r0p1`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip/include`, `src/kernel/dev/arch/at91/softpack-lib/samv71/libchip/include/samv71`, `src/kernel/dev/arch/at91/softpack-lib/samv71/samv71-xplained-ultra/libboard`, `src/kernel/dev/arch/at91/softpack-lib/samv71/samv71-xplained-ultra/libboard/include`
  - bibliothèques liées : `$PROJ_DIR$\..\..\..\..\..\..\..\..\..\lepton\rootstock-mainline\depots\lepton\root\master\sched\embos\scion\sys\root\src\kernel\core\ucore\embOSCXM7_430\Lib\os7m_tl__sp.a`, `building/output/bsp_samv71xplained_ultra_7.80/freertos-debug/Lib/bsp_samv71xplained_ultra_7.80.a`, `building/output/dev_at91samv7x_7.80/samv71-freertos-debug/Lib/dev_at91samv7x_7.80.a`, `building/output/tauon_7.80/tauon-kernel-cortex-m7-debug/Lib/tauon_7.80.a`, `sys/root/src/kernel/core/ucore/embOSCXM7_430/Lib/os7m_tlv_sp.a`

### `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_cmsis/tauon-basic_cmsis_6.21.ewp`

fileVersion 2 ; carte : générique Cortex-M (LM3S) ; workspaces : `tauon-basic_cmsis_6.21.eww` ; fichiers déclarés : 39 ; absents : 16.

- **Debug** — Cortex-M3 (puce), FPU None (FPU=0) ; puce `LM3SxDxx \| TexasInstruments LM3SxDxx` ; EW 6.21.1.52845 ; sources c/cpp/asm 14/0/0, exclus 9, absents 9 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_cmsis/generic_cortex.icf`
  - defines : `OS_LIBMODE_SP` `DEBUG=1` `ewarm`
  - includes : `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/CMSIS/Include`, `src/kernel/core/ucore/cmsis/Device/ARM/ARMCM3/Include`
  - bibliothèques liées : `sys/root/prj/iar/arch/arm/tauon/lib-debug/lib/tauon.a`, `sys/root/src/kernel/core/ucore/embOSCXM3_382/lib/os7m_tl__sp.a`
- **Release** — Cortex-M3 (puce), FPU None (FPU=0) ; puce `LM3SxBxx \| TexasInstruments LM3SxBxx` ; EW 5.41.0.51757 ; sources c/cpp/asm 18/0/1, exclus 0, absents 12 ; ICF par défaut EWARM (`$TOOLKIT_DIR$\CONFIG\generic_cortex.icf`)
  - includes : `sys/user/tauon-basic/prj/iar/Inc (absent)`
  - bibliothèques liées : `sys/root/prj/iar/arch/arm/tauon/lib-debug/lib/tauon.a`, `sys/root/src/kernel/core/ucore/embOSCXM3_382/lib/os7m_tl__sp.a`

### `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_lm3s/tauon-basic_lm3s_6.21.ewp`

fileVersion 2 ; carte : Stellaris LM3S ; workspaces : `tauon-basic_lm3s_6.21.eww` ; fichiers déclarés : 68 ; absents : 23.

- **Debug** — Cortex-M3 (puce), FPU None (FPU=0) ; puce `LM3SxDxx \| TexasInstruments LM3SxDxx` ; EW 6.21.1.52845 ; sources c/cpp/asm 19/0/0, exclus 32, absents 9 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_lm3s/generic_cortex.icf`
  - defines : `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `DEBUG=1` `ewarm` `PART_LM3S9B96`
  - includes : `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`
  - bibliothèques liées : `sys/root/prj/iar/arch/arm/dev/lm3s/Debug/Exe/dev_lm3s.a`, `sys/root/prj/iar/arch/arm/tauon/lib-debug/lib/tauon.a`, `sys/root/src/kernel/core/ucore/embOSCXM3_382/lib/os7m_tl__sp.a`
- **Release** — Cortex-M3 (puce), FPU None (FPU=0) ; puce `LM3SxBxx \| TexasInstruments LM3SxBxx` ; EW 5.41.0.51757 ; sources c/cpp/asm 42/0/0, exclus 0, absents 18 ; ICF par défaut EWARM (`$TOOLKIT_DIR$\CONFIG\generic_cortex.icf`)
  - includes : `sys/user/tauon-basic/prj/iar/Inc (absent)`
  - bibliothèques liées : `sys/root/prj/iar/arch/arm/dev/lm3s/Debug/Exe/dev_lm3s.a`, `sys/root/prj/iar/arch/arm/tauon/lib-debug/lib/tauon.a`, `sys/root/src/kernel/core/ucore/embOSCXM3_382/lib/os7m_tl__dt.a`, `sys/root/src/kernel/core/ucore/embOSCXM3_382/lib/os7m_tl__sp.a`, `sys/root/src/kernel/core/ucore/embOSCXM3_384/lib/os7m_tl__dt.a`, `sys/root/src/kernel/core/ucore/embOSCXM3_384/lib/os7m_tl__r.a`, `sys/root/src/kernel/core/ucore/embOSCXM3_384/lib/os7m_tl__sp.a`

### `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f4-olimex_p407/tauon-basic_stm32f4-olimex_p407_7.20.ewp`

fileVersion 2 ; carte : Olimex STM32-P407 ; workspaces : `tauon-basic_stm32f4-olimex_p407_7.20.eww` ; fichiers déclarés : 67 ; absents : 10.

- **Debug** — Cortex-M4 (puce), FPU None (xcl-corrélé) ; puce `STM32F407ZG \| ST STM32F407ZG` ; EW 7.80.4.12487 ; sources c/cpp/asm 26/0/1, exclus 19, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f4-olimex_p407/FLASH.icf`
  - defines : `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG=1` `ewarm` `PART_LM3S9B96`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM4_386/arch/cmsis/DeviceSupport`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/Device/st/stm32f4xx`
  - bibliothèques liées : `building/output/bsp_olimex_p407_7.30/Debug/Exe/bsp_olimex_p407_7.30.a`, `building/output/dev_stm32f4xx_7.20/Debug/Lib/dev_stm32f4xx_7.20.a`, `building/output/tauon_7.80/tauon-kernel-cortex-m4-debug/Lib/tauon_7.80.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_386/Lib/os7m_tl__sp.a`
- **Release** — Cortex-M3 (puce), FPU None (xcl-corrélé) ; puce `LM3S9B96 \| TexasInstruments LM3S9B96` ; EW 7.80.4.12487 ; sources c/cpp/asm 36/0/2, exclus 0, absents 6 ; ICF par défaut EWARM (`$TOOLKIT_DIR$\CONFIG\generic_cortex.icf`)
  - includes : `sys/user/tauon-basic/prj/iar/Inc (absent)`
  - bibliothèques liées : `building/output/bsp_olimex_p407_7.30/Debug/Exe/bsp_olimex_p407_7.30.a`, `building/output/dev_stm32f4xx_7.20/Debug/Lib/dev_stm32f4xx_7.20.a`, `building/output/tauon_7.80/tauon-kernel-cortex-m4-debug/Lib/tauon_7.80.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_386/Lib/os7m_tl__sp.a`
- **freertos-debug** — Cortex-M3 (puce), FPU None (xcl-corrélé) ; puce `STM32F207ZG \| ST STM32F207ZG` ; EW 7.80.4.12487 ; sources c/cpp/asm 20/0/1, exclus 25, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f4-olimex_p407/FLASH.icf`
  - defines : `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG=1` `ewarm` `PART_LM3S9B96`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM4_386/arch/cmsis/DeviceSupport`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/Device/st/stm32f4xx`
  - bibliothèques liées : `building/output/bsp_olimex_p407_7.30/Debug/Exe/bsp_olimex_p407_7.30.a`, `building/output/dev_stm32f4xx_7.20/Debug/Lib/dev_stm32f4xx_7.20.a`, `building/output/tauon_7.80/tauon-kernel-cortex-m4-debug/Lib/tauon_7.80.a`

### `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f407-discovery/tauon-basic_stm32f407_discovery_7.20.ewp`

fileVersion 2 ; carte : STM32F4-Discovery ; workspaces : `tauon-basic_stm32f407_discovery_7.20.eww` ; fichiers déclarés : 67 ; absents : 10.

- **Debug** — Cortex-M4 (puce), FPU None (xcl-corrélé) ; puce `STM32F407VG \| ST STM32F407VG` ; EW 7.80.4.12487 ; sources c/cpp/asm 22/0/1, exclus 25, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f407-discovery/FLASH.icf`
  - defines : `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG=1` `ewarm` `PART_LM3S9B96`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM4_386/arch/cmsis/DeviceSupport`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/Device/st/stm32f4xx`
  - bibliothèques liées : `building/output/bsp_discovery_f4_7.30/Debug/Exe/bsp_discovery_f4_7.30.a`, `building/output/dev_stm32f4xx_7.20/Debug/Lib/dev_stm32f4xx_7.20.a`, `building/output/tauon_7.80/tauon-kernel-cortex-m4-debug/Lib/tauon_7.80.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_386/Lib/os7m_tl__sp.a`
- **Release** — Cortex-M3 (puce), FPU None (xcl-corrélé) ; puce `LM3S9B96 \| TexasInstruments LM3S9B96` ; EW 7.80.4.12487 ; sources c/cpp/asm 36/0/2, exclus 0, absents 6 ; ICF par défaut EWARM (`$TOOLKIT_DIR$\CONFIG\generic_cortex.icf`)
  - includes : `sys/user/tauon-basic/prj/iar/Inc (absent)`
  - bibliothèques liées : `building/output/bsp_discovery_f4_7.30/Debug/Exe/bsp_discovery_f4_7.30.a`, `building/output/dev_stm32f4xx_7.20/Debug/Lib/dev_stm32f4xx_7.20.a`, `building/output/tauon_7.80/tauon-kernel-cortex-m4-debug/Lib/tauon_7.80.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_386/Lib/os7m_tl__sp.a`
- **freertos-debug** — Cortex-M3 (puce), FPU None (xcl-corrélé) ; puce `STM32F207ZG \| ST STM32F207ZG` ; EW 7.80.4.12487 ; sources c/cpp/asm 20/0/1, exclus 25, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f407-discovery/FLASH.icf`
  - defines : `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG=1` `ewarm` `PART_LM3S9B96`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM4_386/arch/cmsis/DeviceSupport`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/Device/st/stm32f4xx`
  - bibliothèques liées : `building/output/bsp_discovery_f4_7.30/Debug/Exe/bsp_discovery_f4_7.30.a`, `building/output/dev_stm32f4xx_7.20/Debug/Lib/dev_stm32f4xx_7.20.a`, `building/output/tauon_7.80/tauon-kernel-cortex-m4-debug/Lib/tauon_7.80.a`

### `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f469i-eval/tauon-basic_stm32f469i_eval_7.80.ewp`

fileVersion 2 ; carte : STM32F469I-EVAL ; workspaces : `tauon-basic_stm32f469i_eval_7.80.eww` ; fichiers déclarés : 45 ; absents : 10.

- **Debug** — Cortex-M4 (puce), FPU None (xcl-corrélé) ; puce `STM32F469NI \| ST STM32F469NI` ; EW 7.80.4.12487 ; sources c/cpp/asm 10/0/1, exclus 18, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f469i-eval/FLASH.icf`
  - defines : `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG=1` `ewarm` `PART_LM3S9B96`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM4_386/arch/cmsis/DeviceSupport`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/Device/st/stm32f4xx`
  - bibliothèques liées : `building/output/bsp-stm32f469i-eval/Debug/Exe/bsp-stm32f469i-eval.a`, `building/output/dev_stm32f4xx_7.20/Debug/Lib/dev_stm32f4xx_7.20.a`, `building/output/tauon_7.80/tauon-kernel-cortex-m4-debug/Lib/tauon_7.80.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_386/Lib/os7m_tl__sp.a`
- **Release** — Cortex-M3 (puce), FPU None (xcl-corrélé) ; puce `LM3S9B96 \| TexasInstruments LM3S9B96` ; EW 7.80.4.12487 ; sources c/cpp/asm 19/0/2, exclus 0, absents 6 ; ICF par défaut EWARM (`$TOOLKIT_DIR$\CONFIG\generic_cortex.icf`)
  - includes : `sys/user/tauon-basic/prj/iar/Inc (absent)`
  - bibliothèques liées : `building/output/bsp-stm32f469i-eval/Debug/Exe/bsp-stm32f469i-eval.a`, `building/output/dev_stm32f4xx_7.20/Debug/Lib/dev_stm32f4xx_7.20.a`, `building/output/tauon_7.80/tauon-kernel-cortex-m4-debug/Lib/tauon_7.80.a`, `sys/root/src/kernel/core/ucore/embOSCXM4_386/Lib/os7m_tl__sp.a`
- **freertos-debug** — Cortex-M3 (puce), FPU None (xcl-corrélé) ; puce `STM32F207ZG \| ST STM32F207ZG` ; EW 7.80.4.12487 ; sources c/cpp/asm 6/0/1, exclus 21, absents 3 ; ICF `sys/user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f469i-eval/FLASH.icf`
  - defines : `OS_LIBMODE_SP` `_OS_LIBMODE_DT` `_OS_LIBMODE_R` `NDEBUG=1` `ewarm` `PART_LM3S9B96`
  - includes : `src/kernel/core/ucore/freeRTOS_8-0-0/source/include`, `src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F`, `src/kernel/core/ucore/embOSCXM4_386/Inc`, `src/kernel/core/ucore/embOSCXM4_386/arch/cmsis/DeviceSupport`, `src/kernel/core/ucore/embOSCXM3_382/inc (absent)`, `sys/root/src`, `src/kernel/net/lwip`, `src/kernel/net/lwip/include`, `src/kernel/net/lwip/ports/arm`, `src/kernel/net/lwip/ports/arm/include`, `src/kernel/net/lwip/include/ipv4 (absent)`, `src/kernel/core/ucore/cmsis`, `src/kernel/core/ucore/cmsis/Device/st/stm32f4xx`
  - bibliothèques liées : `building/output/bsp-stm32f469i-eval/Debug/Exe/bsp-stm32f469i-eval.a`, `building/output/dev_stm32f4xx_7.20/Debug/Lib/dev_stm32f4xx_7.20.a`, `building/output/tauon_7.80/tauon-kernel-cortex-m4-debug/Lib/tauon_7.80.a`

