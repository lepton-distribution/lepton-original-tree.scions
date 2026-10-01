# Handoff étape 4, module `kernel/net` → modules suivants

État au 2026-10-01 : module fait sur `migration/etape-4-kernel-net`, validé par l'utilisateur
le 2026-10-01 et fusionné. Module suivant : `lib`. Procédure générale : `handoff/etape-4-outillage.md`.

## Résultat du module
- **Rien à porter** : les 376 fichiers de `kernel/net` (lwIP, uIP, uIP 2.5, y compris
  `lwip/ports/arm`) sont du code tiers par leur en-tête ; `transform_iar.py` : 0 occurrence ;
  aucune directive d'ISA ; mass_compile 38/38 inchangé.
- Dette « LWIP_PROVIDE_ERRNO » corrigée (décision utilisateur du 2026-10-01) : `lwipopts.h` utilise
  `LWIP_ERRNO_INCLUDE "kernel/core/errno.h"` ; l'errno des sockets vu par les applications est
  dans la numérotation Lepton (connect refusé : 104 → ECONNRESET=15) ; avertissements des fichiers
  socket 77 → 1 (`struct _reent` de `RTOS.h`, dette de l'étape 3).
- Test : `net.ping_ftpd` exige aussi l'errno d'un `connect()` refusé (`/usr/sbin/net/tsterrno`,
  valeurs lues dans `kernel/core/errno.h`) ; rouge avant la correction, vert après.

## Décisions actées pendant le module (2026-10-01)
- Correction de l'errno lwIP dans ce module, avec test (exception étroite à D1a : `lwipopts.h`,
  fichier de configuration de lwIP déjà adapté par Lepton).
- Défaut du rootfs découvert (ci-dessous) : contourné (`tsterrno` dans `/usr/sbin/net`), traité à
  part avec son propre test.

## Artefacts produits (manifeste)
| Fichier | Contenu, quand le lire |
|---|---|
| `scion/sys/root/src/sbin/net/tsterrno.c` | pseudo-binaire de test : errno d'un `connect()` |
| `scion/tests/net_qemu.py` | + vérification errno (`--errno-header`, `--refused-port`) ; `except` corrigé |
| `mkconf_tauon_basic_qemu_mps2_an386.xml` | + `tsterrno` dans `/usr/sbin/net` |

## Écarts au plan et pièges découverts
- **Défaut préexistant du rootfs (à traiter, module `tools/mklepton` ou session dédiée)** : le
  mkconf QEMU déclare `bin/net` sans `bin` ; mklepton fait `mkdir /usr/bin/net` alors que
  `/usr/bin` n'existe pas, l'appel réussit, `ls /usr/bin` montre `net` mais `ls /usr/bin/net`
  est vide (déjà sur le firmware validé de l'étape 3). Avec une seule entrée, `ftpd` se lance
  par chance ; avec deux, `bin/net/ftpd` exécute l'autre binaire. Cause à établir : VFS/UFS
  (`mkdir` sans parent qui réussit) ou mklepton (répertoires intermédiaires). Toute carte dont le
  mkconf a des `dest_path` à plusieurs niveaux est concernée (Olimex P407 : à vérifier, étape 5).
- mklepton ne lit pas un commentaire XML placé dans un élément `<binaries>` (« Parse error »).
- `tests/net_qemu.py` : `except (ftplib.all_errors, OSError)` levait TypeError et masquait les
  erreurs FTP ; corrigé.
- Le hook `lepton_guard.py` évalue les redirections depuis le répertoire courant du shell : un
  `cd` dans le trunk persiste d'une commande à l'autre ; revenir dans le clone.
- `ORIGINE_TIERS` couvre `lwip`/`uip` en entier : ici, vérifié, aucun fichier sous licence Lepton.

## Non transmis volontairement
- Valeurs errno Linux/Lepton comparées : commit `lwIP : errno des sockets…` et
  `kernel/net/lwip/include/lwip/errno.h` / `kernel/core/errno.h`.
