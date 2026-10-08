# Étape 9 — `.bin` construits sur macOS, comparés à Debian (mesure, sans critère)

Mac Intel, macOS 15.8.1 ; Arm GNU Toolchain 14.2.Rel1 `darwin-x86_64` (GCC 14.2.1, newlib
« 4.4.0 ») ; `SOURCE_DATE_EPOCH=0 ci/run.sh` après `clean`, 2026-10-08, branche `migration/etape-9`
(sources ARM identiques à la fin de l'étape 8 : seuls `cmake/isa/host.cmake` et le test `net`
ont changé). Référence : tableau de `handoff/etape-8.md` (Debian, newlib 4.5.0.20241231).

**Résultat : les 14 `lepton.bin` diffèrent de Debian** (écart newlib accepté le 2026-10-08) ;
firmwares QEMU validés par exécution (fumée, banc KAL), cartes à l'étape 10.

Taille (`arm-none-eabi-size lepton.elf`) et répartition par origine
(`tools/migration/map_origine.py`, octets text/data/bss). Comparaison par section : rejouer
`map_origine.py` sur les artefacts Debian du même commit et confronter les colonnes ; newlib
et libgcc/crt sont les origines attendues de l'écart.

| Preset | SHA-256 macOS (`lepton.bin`) | text / data / bss | Répartition par origine |
|---|---|---|---|
| nucleo-f439zi-embos | `d01dfb6cb557b345…` | 281864 / 1064 / 143824 | embos:34690/52/7307 ; lepton:253498/909/136102 ; libgcc/crt:2908/0/25 ; newlib:10388/92/330 |
| nucleo-f439zi-freertos | `0244719f89b45984…` | 288560 / 1188 / 155372 | lepton:294100/1085/154964 ; libgcc/crt:3848/0/25 ; newlib:10380/92/330 |
| nucleo-wl55jc1-embos | `c271bf73e678fa9c…` | 109056 / 840 / 25528 | embos:27437/52/7003 ; lepton:79998/682/18134 ; libgcc/crt:3608/0/25 ; newlib:8118/92/330 |
| nucleo-wl55jc1-freertos | `98033109b62d621f…` | 114424 / 844 / 32172 | lepton:112938/738/31780 ; libgcc/crt:3608/0/25 ; newlib:8118/92/330 |
| qemu-mps2-an386-embos | `303c40bc889c7154…` | 275044 / 1444 / 128200 | embos:34698/52/7307 ; lepton:246223/1289/120482 ; libgcc/crt:2908/0/25 ; newlib:10388/92/330 |
| qemu-mps2-an386-embos-soft | `2fecf40b4219f438…` | 274756 / 1444 / 127656 | embos:34316/52/7035 ; lepton:245489/1289/120210 ; libgcc/crt:2872/0/25 ; newlib:9980/92/330 |
| qemu-mps2-an386-freertos | `440666c05537696e…` | 281804 / 1448 / 139752 | lepton:286885/1345/139344 ; libgcc/crt:3848/0/25 ; newlib:10380/92/330 |
| qemu-mps2-an386-freertos-soft | `be7ce76f0b7ea94d…` | 281452 / 1448 / 139192 | lepton:285775/1345/138784 ; libgcc/crt:3798/0/25 ; newlib:9972/92/330 |
| qemu-mps2-an500-embos | `456a3e6f61b592d7…` | 272244 / 1444 / 128200 | embos:34656/52/7307 ; lepton:245693/1289/120482 ; libgcc/crt:272/0/25 ; newlib:10254/92/330 |
| qemu-mps2-an500-freertos | `222f10d67cf0eeed…` | 278988 / 1448 / 139752 | lepton:286285/1345/139344 ; libgcc/crt:1206/0/25 ; newlib:10246/92/330 |
| samd21-xplained-pro-embos | `dc2dc432f0c3834c…` | 94092 / 924 / 13008 | embos:27766/52/4939 ; lepton:58157/769/7682 ; libgcc/crt:9454/0/25 ; newlib:8059/92/330 |
| samd21-xplained-pro-freertos | `a60f0f107cc4478e…` | 98724 / 928 / 17472 | lepton:90683/825/17095 ; libgcc/crt:9454/0/25 ; newlib:8059/92/330 |
| stm32f746g-disco-embos | `bbbd0a903edcbfe8…` | 280936 / 1180 / 155640 | embos:35196/52/7307 ; lepton:251016/1025/134838 ; libgcc/crt:3848/0/25 ; newlib:10380/92/330 |
| stm32f746g-disco-freertos | `4e83cad864b302e6…` | 286100 / 1184 / 166784 | lepton:291414/1081/153700 ; libgcc/crt:3848/0/25 ; newlib:10380/92/330 |

Empreintes complètes : `$LEPTON_BUILD/etape9-bin-sha256.txt` (Mac) ; relevé mémoire :
`$LEPTON_BUILD/ci/memoire.csv` (seuil 90 % respecté ; CCM du F439 sous FreeRTOS : 86 %).
