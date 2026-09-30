# Annexe — Ajout d'un nouveau cœur : RISC-V (reporté)

## Statut

**Reporté, hors de la séquence 0-7.** Le portage RISC-V n'est pas planifié. Ce qui est planifié,
c'est que l'architecture de sources et de compilation l'accueille sans refonte : c'est une exigence
des étapes 2 (axes ISA / cœur / carte / micro-noyau, `doc/migration/ajout-coeur.md`) et 4 (KAL
`arch/<famille>/`). Cette annexe sert de liste de contrôle pour la revue de ces étapes et de point
de départ le jour où le portage sera lancé (il deviendra alors une étape à part entière, rédigée
avec le skill `lepton-portage-instructions`).

## Ce que les étapes 2 et 4 doivent garantir (revue à l'étape 6)

- [ ] Une famille `rv32` s'ajoute par : `cmake/toolchains/rv32-gcc.cmake`, `cmake/cpu/rv32imac.cmake`,
      `cmake/boards/qemu-virt-rv32.cmake`, `ld/mem_qemu-virt-rv32.ld`, `kal/arch/rv32/`, démarrage et
      trap en asm dans le répertoire d'architecture de la famille, une ligne du dispatcher `kal.h`.
- [ ] `ld/common-*.ld` : sections communes séparables de la partie propre à ARM (`.ARM.exidx`).
- [ ] Aucune hypothèse Cortex-M dans le code commun : NVIC, SVC, PendSV, SysTick, CMSIS restent
      dans `arch/armv*` et dans les BSP ARM.
- [ ] Sections critiques par macros neutres (`__lepton_disable_irq`…).
- [ ] Offsets de contexte générés (`asm-offsets`), jamais codés en dur.
- [ ] Banc KAL paramétré par machine QEMU, pas lié à `qemu-system-arm`.

## Éléments techniques déjà établis (pour le futur portage)

- Cible : RV32IMAC, mode machine, sans MMU ; toolchain `riscv64-unknown-elf` (multilib rv32) ou
  xPack `riscv-none-elf-gcc`, à épingler.
- Micro-noyau : paquet embOS RISC-V déjà téléchargé ; vérifier la carte cible dans ses BSP.
- Appel système : `ecall` (`mcause` = 11 en mode M), un seul vecteur `mtvec` qui démultiplexe
  exceptions, `ecall` et interruptions.
- Contrôleur d'interruptions : CLINT + PLIC (QEMU `virt`) ou CLIC/ECLIC selon le MCU (GD32VF103 :
  ECLIC).
- QEMU : `qemu-system-riscv32 -M virt -bios none`, démarrage en mode M à `0x80000000`, UART 16550.
- Pièges : `gp` chargé sous `.option norelax` ; alignement de `mtvec` ; aucun empilement matériel des
  registres (contexte entièrement logiciel) ; extension A requise ou non pour les atomiques.
- Candidats matériels : GD32VF103 (carte d'évaluation, Longan Nano), SiFive HiFive1, ESP32-C3
  (support embOS à confirmer).
