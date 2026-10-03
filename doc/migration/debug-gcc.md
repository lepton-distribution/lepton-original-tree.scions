# Flash et débogage GCC — NUCLEO-F439ZI (étape 5)

Sonde ST-LINK/V2-1 intégrée (USB direct, OpenOCD 0.12), `gdb-multiarch`. Fichiers :
`debug/openocd-nucleo-f439zi.cfg` (reprend `board/st_nucleo_f4.cfg`), `debug/gdbinit-nucleo-f439zi`.
Un seul client de la sonde à la fois : arrêter tout `openocd` lancé avant un `ctest -L board` ou
une cible `flash`.

## Flash

```bash
cmake --build --preset nucleo-f439zi-embos --target flash
# équivalent : openocd -f debug/openocd-nucleo-f439zi.cfg -c "program <lepton.elf> verify reset exit"
```

## Session gdb

```bash
openocd -f debug/openocd-nucleo-f439zi.cfg                      # terminal 1 (gdb 3333, telnet 4444)
gdb-multiarch -x debug/gdbinit-nucleo-f439zi "$LEPTON_BUILD/nucleo-f439zi-embos/lepton.elf"
(gdb) lepton-load        # flash + reset halt
(gdb) break main
(gdb) continue
```

Six points d'arrêt matériels au plus (le code est en flash). `monitor reset halt` revient au
vecteur de reset ; `monitor reset run` relance sans débogueur actif.

L'attachement de gdb arrête le cœur. Terminer toute session (et tout `gdb -batch`) par
`monitor resume` avant `detach` : sans cela, le cœur a été trouvé resté arrêté après l'arrêt
d'OpenOCD (`DHCSR` `0x00030003`), la carte figée (tick, réseau, console), ce qui imite un défaut
du firmware. Relance sans gdb : `openocd -f debug/openocd-nucleo-f439zi.cfg -c init -c halt -c
resume -c exit` (le `halt` met à jour l'état connu d'OpenOCD ; un `resume` seul est refusé).

## Faute

Lire les registres de faute avant toute autre hypothèse (`lepton-fault` dans gdb) :

| Registre | Adresse | Lecture |
|---|---|---|
| CFSR | `0xE000ED28` | MMFSR [7:0], BFSR [15:8], UFSR [31:16] (ex. `0x00020000` = INVSTATE) |
| HFSR | `0xE000ED2C` | `0x40000000` = faute escaladée (FORCED) |
| MMFAR / BFAR | `0xE000ED34` / `0xE000ED38` | adresse fautive si MMARVALID / BFARVALID |

Le gestionnaire par défaut (`Default_Handler`) boucle : `IPSR` (`$xPSR & 0x1ff`) donne l'exception,
`LR` (EXC_RETURN) la pile du contexte interrompu (`0xFFFFFFFD` : PSP, cadre de base), dont les huit
mots sont R0-R3, R12, LR, PC, xPSR. Vérifier dans xPSR le bit T (24) et l'état ICI/IT
(bits 26:25 et 15:10) : exemple réel de l'étape 5, `validation-nucleo-f439zi.md`.

## Semihosting (banc KAL)

`tests/kal_openocd.py` : `--program kal_bench.elf`, puis `--test=T1` (OpenOCD : `arm semihosting
enable`, ligne de commande, sortie, code de retour). Sans débogueur, un `BKPT 0xAB` met le cœur en
faute : `kal_bench` ne tourne que sous ce lanceur.

## Lecture de registres, carte en marche

OpenOCD lit la mémoire sans arrêter le cœur : `openocd -f debug/openocd-nucleo-f439zi.cfg -c init
-c "mdw 0x40028168" -c exit` (ici le compteur de trames émises du MAC Ethernet). Les registres du
PHY se lisent par MDIO (`MACMIIAR` `0x40028010`, `MACMIIDR` `0x40028014`, adresse PHY 0).
