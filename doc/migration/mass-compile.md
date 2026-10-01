# Compilation de masse du périmètre actif — tableau de bord

Généré par `tools/migration/mass_compile.sh` — ne pas éditer à la main.  
Commande : `mass_compile.sh --audit-csv doc/migration/audit-iar.csv --report doc/migration/mass-compile.md --csv doc/migration/mass-compile.csv -q`

Résultat : **243/348 fichiers C OK (69.8 %)** ; `arm-none-eabi-gcc -c` (outils hôte : `cc -m32`).

Commande de chaque fichier : entrée du preset qui le compile, sinon gabarit du preset QEMU le plus proche corrigé par profil (voir l'en-tête du script). Détail par fichier : `mass-compile.csv`.

## Par module

| Module | OK / total | IAR-ismes (Lepton) | IAR-ismes (tiers) |
|---|---|---:|---:|
| kernel/core | 47 / 50 | 19 | 38 |
| kernel/dev | 65 / 154 | 15 | 16 |
| kernel/fs | 23 / 26 | 0 | 4 |
| kernel/net | 38 / 38 | 0 | 0 |
| lib | 27 / 27 | 0 | 0 |
| sbin | 32 / 35 | 0 | 0 |
| bin | 3 / 8 | 0 | 0 |
| tauon-basic | 7 / 9 | 32 | 0 |
| tools/mklepton | 1 / 1 | 4 | 0 |

## Histogramme des erreurs (première erreur de chaque fichier)

| Fichiers | Erreur normalisée | Exemple |
|---:|---|---|
| 71 | `Legacy/stm32_hal_legacy.h: No such file or directory` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal.c` |
| 13 | `expected 'X' before 'X'` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_eth.c` |
| 5 | `implicit declaration of function 'X'; did you mean 'X'? [-Wimplicit-function-declaration]` | `sys/root/src/bin/net/httpc/httpc.c` |
| 5 | `implicit declaration of function 'X' [-Wimplicit-function-declaration]` | `sys/root/src/bin/net/mongoose/mongoose.c` |
| 5 | `static declaration of 'X' follows non-static declaration` | `sys/root/src/kernel/core/net/modem_core/modem_core.c` |
| 4 | `passing argument N of 'X' from incompatible pointer type [-Wincompatible-pointer-types]` | `sys/root/src/bin/net/telnetd.c` |
| 1 | `conflicting types for 'X'; have 'X'` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/eth.c` |
| 1 | `unknown type name 'X'` | `sys/root/src/kernel/fs/fatfs/core/diskio.c` |

## Fichiers en échec

| Fichier | Première erreur |
|---|---|
| `sys/root/src/bin/net/httpc/httpc.c` | `sys/root/src/bin/net/httpc/httpc.c:223: implicit declaration of function 'perror'; did you mean 'error'? [-Wimplicit-function-declaration]` |
| `sys/root/src/bin/net/mongoose/mongoose.c` | `sys/root/src/bin/net/mongoose/mongoose.c:521: implicit declaration of function 'tolower' [-Wimplicit-function-declaration]` |
| `sys/root/src/bin/net/mongoose/mongoosed.c` | `sys/root/src/bin/net/mongoose/mongoosed.c:288: implicit declaration of function 'strerror' [-Wimplicit-function-declaration]` |
| `sys/root/src/bin/net/telnetd.c` | `sys/root/src/bin/net/telnetd.c:105: passing argument 3 of 'libc_accept' from incompatible pointer type [-Wincompatible-pointer-types]` |
| `sys/root/src/bin/test2.c` | `sys/root/src/bin/test2.c:237: passing argument 3 of 'libc_accept' from incompatible pointer type [-Wincompatible-pointer-types]` |
| `sys/root/src/kernel/core/net/modem_core/modem_core.c` | `sys/root/src/kernel/core/net/modem_core/modem_core.c:766: static declaration of 'modem_core_mq_post_unconnected_request' follows non-static declaration` |
| `sys/root/src/kernel/core/net/modem_core/modem_core_socket.c` | `sys/root/src/kernel/core/net/modem_core/modem_core_socket.c:145: static declaration of 'modem_core_socket_socket' follows non-static declaration` |
| `sys/root/src/kernel/core/net/uip_core/uip_core_socket.c` | `sys/root/src/kernel/core/net/uip_core/uip_core_socket.c:243: static declaration of 'uip_core_socket_socket' follows non-static declaration` |
| `sys/root/src/kernel/dev/arch/all/debug/dev_os_debug.c` | `sys/root/src/kernel/dev/arch/all/debug/dev_os_debug.c:105: passing argument 1 of 'OS_COM_SetRxCallback' from incompatible pointer type [-Wincompatible-pointer-types]` |
| `sys/root/src/kernel/dev/arch/all/i2c/rtc/dev_rtc_nxp_pca8565.c` | `sys/root/src/kernel/dev/arch/all/i2c/rtc/dev_rtc_nxp_pca8565.c:215: static declaration of 'dev_rtc_nxp_pca8565_settime' follows non-static declaration` |
| `sys/root/src/kernel/dev/arch/all/modem/ublox/dev_modem_ublox_sarag3.c` | `sys/root/src/kernel/dev/arch/all/modem/ublox/dev_modem_ublox_sarag3.c:916: implicit declaration of function 'atof'; did you mean 'atol'? [-Wimplicit-function-declaration]` |
| `sys/root/src/kernel/dev/arch/all/ppp/dev_ppp_uip/dev_ppp_uip.c` | `sys/root/src/kernel/dev/arch/all/ppp/dev_ppp_uip/dev_ppp_uip.c:223: static declaration of 'dev_ppp_uip_load' follows non-static declaration` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_adc.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_adc_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_can.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_cec.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_cortex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_crc.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_cryp.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_cryp_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_dac.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_dac_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_dcmi.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_dcmi_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_dma.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_dma2d.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_dma_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_eth.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_flash.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_flash_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_flash_ramfunc.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_fmpi2c.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_fmpi2c_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_gpio.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_hash.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_hash_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_hcd.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_i2c.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_i2c_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_i2s.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_i2s_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_irda.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_iwdg.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_ltdc.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_msp_template.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_nand.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_nor.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_pccard.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_pcd.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_pcd_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_pwr.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_pwr_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_qspi.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_rcc.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_rcc_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_rng.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_rtc.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_rtc_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_sai.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_sai_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_sd.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_sdram.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_smartcard.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_spdifrx.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_spi.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_sram.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_tim.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_tim_ex.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_uart.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_usart.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_hal_wwdg.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_ll_fmc.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_ll_fsmc.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_ll_sdmmc.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/stm32f4xx_ll_usb.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_cpu_x.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_cubemx_eth.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_dac_x.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_eth.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:58: expected ';' before 'union'` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_flash.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_i2c_x.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_sdio.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_spi_x.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:58: expected ';' before 'union'` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_uart_x.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:58: expected ';' before 'union'` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/eth.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:51: conflicting types for 's32'; have 'int'` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/flash_if.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h:49: Legacy/stm32_hal_legacy.h: No such file or directory` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/gpio.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:58: expected ';' before 'union'` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/spi.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:58: expected ';' before 'union'` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/uart.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:58: expected ';' before 'union'` |
| `sys/root/src/kernel/dev/bsp/discovery_f4/dev_discovery_f4_board/dev_discovery_f4_board.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:58: expected ';' before 'union'` |
| `sys/root/src/kernel/dev/bsp/discovery_f4/dev_discovery_f4_peripherals/dev_discovery_f4_uart_6.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:58: expected ';' before 'union'` |
| `sys/root/src/kernel/dev/bsp/olimex_p407/dev_olimex_p407_board/dev_olimex_p407_board.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:58: expected ';' before 'union'` |
| `sys/root/src/kernel/dev/bsp/olimex_p407/dev_olimex_p407_peripherals/dev_olimex_p407_spi_3.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:58: expected ';' before 'union'` |
| `sys/root/src/kernel/dev/bsp/olimex_p407/dev_olimex_p407_peripherals/dev_olimex_p407_uart_3.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:58: expected ';' before 'union'` |
| `sys/root/src/kernel/dev/bsp/stm32f469i-eval/dev_stm32f469i_eval_board/dev_stm32f469i_eval_board.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:58: expected ';' before 'union'` |
| `sys/root/src/kernel/dev/bsp/stm32f469i-eval/dev_stm32f469i_eval_peripherals/dev_stm32f469i_eval_usart_1.c` | `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h:58: expected ';' before 'union'` |
| `sys/root/src/kernel/fs/fat/fatcore_msdos.c` | `sys/root/src/kernel/fs/fat/fatcore_msdos.c:248: implicit declaration of function 'strtok_r'; did you mean 'strtok'? [-Wimplicit-function-declaration]` |
| `sys/root/src/kernel/fs/fatfs/core/diskio.c` | `sys/root/src/kernel/fs/fatfs/core/diskio.c:158: unknown type name '__weak'` |
| `sys/root/src/kernel/fs/fatfs/fatfscore.c` | `sys/root/src/kernel/fs/fatfs/fatfscore.c:662: passing argument 1 of 'f_close' from incompatible pointer type [-Wincompatible-pointer-types]` |
| `sys/root/src/sbin/btb.c` | `sys/root/src/sbin/btb.c:462: implicit declaration of function 'atof'; did you mean 'atol'? [-Wimplicit-function-declaration]` |
| `sys/root/src/sbin/stty.c` | `sys/root/src/sbin/stty.c:1158: implicit declaration of function 'toupper' [-Wimplicit-function-declaration]` |
| `sys/root/src/sbin/wrapr.c` | `sys/root/src/sbin/wrapr.c:356: implicit declaration of function 'atof'; did you mean 'atol'? [-Wimplicit-function-declaration]` |
| `sys/user/tauon-basic/src/bin/dhrystone/dhry21b.c` | `sys/user/tauon-basic/src/bin/dhrystone/dhry21b.c:157: implicit declaration of function 'strcmp' [-Wimplicit-function-declaration]` |
| `sys/user/tauon-basic/src/bin/dhrystone/dhrystone_main.c` | `sys/user/tauon-basic/src/bin/dhrystone/dhrystone_main.c:98: implicit declaration of function 'runDhrystone' [-Wimplicit-function-declaration]` |
