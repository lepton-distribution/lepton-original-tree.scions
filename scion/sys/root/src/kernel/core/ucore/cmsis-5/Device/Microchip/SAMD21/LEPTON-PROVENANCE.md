# Provenance (migration Lepton, étape 6)

Microchip `SAMD21_DFP` **3.8.270** (`Microchip.SAMD21_DFP.3.8.270.atpack`,
https://packs.download.microchip.com/, téléchargé le 2026-10-05, SHA-256
`8474d111df8bd2fbe688c509c6515810ec6df18269f5f03a49a66d3e7400bdc3`), Apache-2.0 (`LICENSE.txt`).
Décision utilisateur du 2026-10-05 (plutôt que l'ASF3). Fichiers non modifiés ; sous-ensemble de
`samd21a/include` pour la SAMD21 Xplained Pro (ATSAMD21J18A) : `samd21j18a.h`,
`system_samd21j18a.h`, `component-version.h`, `component/`, `instance/`, `pio/samd21j18a.h`. Les
autres puces, les variantes `samd21b/c/d/l`, les modèles de démarrage et d'édition de liens
(gcc, iar, keil, xc32), SVD et ATDF ne sont pas repris. Ajouter l'en-tête d'une autre puce depuis
le même paquet si une carte l'exige.

CMSIS-Core : `CMSIS/Core/Include/core_cm0plus.h` (V5.0.9, exigé par `samd21j18a.h`) copié sans
modification du paquet embOS-Classic V5.20.0.0 (`Start/BoardSupport/Microchip/
SAMD20J18_SAMD20_XPlainedPro/CoreSupport`), même livraison CMSIS 5.6 que les fichiers déjà
présents (`cmsis_compiler.h`, `cmsis_gcc.h`, `cmsis_version.h`, `mpu_armv7.h` identiques octet à
octet), comme `core_cm7.h` (décision 2026-10-05, module M7 QEMU).
